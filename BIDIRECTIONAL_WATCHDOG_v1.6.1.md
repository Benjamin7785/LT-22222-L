# Bidirectional Watchdog Link Monitoring - v1.6.1

## Overview

Version 1.6.1 extends the watchdog system (introduced in v1.6.0) to support **bidirectional monitoring**: both transmitter and receiver can now monitor the communication link and provide visual indicators when the link is lost.

---

## Architecture

### Unified Watchdog System

The watchdog system automatically adapts based on the device mode (TDC setting):

| Mode | TDC Value | Monitors | Indicator | Action on Link Lost |
|------|-----------|----------|-----------|-------------------|
| **Transmitter** | `TDC > 0` | Receiver ACK/feedback | `DO1 = HIGH` | Visual indicator only |
| **Receiver** | `TDC = 0` | Transmitter heartbeat | `DO2 = HIGH` | Safe state (RO1/RO2=LOW) |

### Key Design Decisions

1. **Same Parameters**: Both transmitter and receiver use the same `interval_seconds` and `max_missed` values
   - Reason: Heartbeat interval is synchronized, so monitoring intervals should match
   - Configuration: Single `AT+WATCHDOG` command configures both sides

2. **Auto-Detection**: No separate configuration needed
   - System automatically detects mode based on `APP_TX_DUTYCYCLE`
   - Transparent to the user

3. **Different Actions**: While monitoring is symmetric, the actions differ
   - **Transmitter**: Only visual indicator (DO1), maintains last RO/DO states
   - **Receiver**: Safe state (RO outputs to LOW) for safety

---

## Implementation Details

### 1. Watchdog Structure

The existing `watchdog_t` structure (from v1.6.0) is reused without modification:

```c
typedef struct {
    bool enabled;                 // Watchdog on/off
    uint8_t interval_seconds;     // Check interval
    uint8_t missed_count;         // Current missed count
    uint8_t max_missed;           // Max misses before triggering
    uint32_t last_received_time;  // Timestamp of last packet
    bool link_active;             // Link status flag
    bool safe_state_active;       // Safe state flag
} watchdog_t;
```

**No separate watchdog needed** - the same structure handles both modes.

### 2. Modified Functions

#### `watchdog_init()` (lines 2424-2457)

**Changes**:
- Added mode detection message based on `APP_TX_DUTYCYCLE`
- Prints "TRANSMITTER - Monitoring receiver ACK (DO1 indicator)" or "RECEIVER - Monitoring transmitter heartbeat (DO2 indicator)"

**Benefits**:
- Clear console output shows which mode is active
- Helps with troubleshooting

#### `watchdog_trigger_safe_state()` (lines 2522-2554)

**Major Changes**:
- Added `if(APP_TX_DUTYCYCLE > 0)` check to detect transmitter mode
- **Transmitter mode**: Sets `DO1 = HIGH` (link lost indicator), keeps RO outputs unchanged
- **Receiver mode**: Sets `RO1/RO2 = LOW` (safe state), sets `DO2 = HIGH` (link lost indicator)

**Code**:
```c
if(APP_TX_DUTYCYCLE > 0)
{
    // TRANSMITTER MODE (TDC > 0)
    PPRINTF("TX: No ACK from receiver - Setting DO1=HIGH (link lost indicator)\r\n");
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);  // DO1 = HIGH
    DO1_flag = 1;
    // Note: RO outputs remain in last known state
}
else
{
    // RECEIVER MODE (TDC = 0)
    PPRINTF("RX: No heartbeat from transmitter - Setting RO1=LOW, RO2=LOW, DO2=HIGH\r\n");
    HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    RO1_flag = 0;
    RO2_flag = 0;
}
```

#### `watchdog_reset()` (lines 2476-2520)

**Major Changes**:
- Added mode detection for recovery
- **Transmitter mode**: Clears `DO1` indicator (sets to LOW), prints "receiver responding"
- **Receiver mode**: Clears `DO2` indicator (sets to LOW), prints "transmitter heartbeat restored"

**Code**:
```c
if(watchdog.safe_state_active)
{
    PPRINTF("\r\n=== LINK RESTORED ===\r\n");
    watchdog.safe_state_active = false;
    watchdog.link_active = true;
    
    if(APP_TX_DUTYCYCLE > 0)
    {
        // TRANSMITTER MODE - Clear DO1 indicator
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
        DO1_flag = 0;
        PPRINTF("TX: DO1 indicator OFF - receiver responding\r\n");
    }
    else
    {
        // RECEIVER MODE - Clear DO2 indicator
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
        PPRINTF("RX: DO2 indicator OFF - transmitter heartbeat restored\r\n");
    }
}
```

**Important Note**: On transmitter, the feedback processing code (lines 946-949 in `RxData()`) automatically restores DO1 to the correct state (mirroring receiver's RO1) immediately after `watchdog_reset()` clears it. This provides smooth transition from "link lost" indicator back to normal operation.

### 3. Feedback Processing Enhancement

Added comment in `RxData()` (line 944) to clarify that DO1 mirroring also handles watchdog recovery:

```c
// NEW ARCHITECTURE - Mirror receiver's RO states to transmitter's DO/RO outputs
// v1.6.1: This also restores DO1 after watchdog link recovery (DO1 was HIGH during link lost)
```

### 4. Documentation Additions

Updated global watchdog comment (lines 194-210) to explain bidirectional monitoring:

```c
// NEW CODE - Watchdog system for bidirectional link monitoring (v1.6.1)
// TRANSMITTER (TDC>0): Monitors receiver ACK/feedback packets
//   - If no ACK received for (interval_seconds × max_missed) → DO1=HIGH (link lost)
//   - When ACK received → DO1=LOW, resume normal operation
// RECEIVER (TDC=0): Monitors transmitter heartbeat packets
//   - If no heartbeat for (interval_seconds × max_missed) → RO1/RO2=LOW, DO2=HIGH (safe state)
//   - When heartbeat received → DO2=LOW, resume normal operation
// Both modes use the SAME watchdog parameters (configured via AT+WATCHDOG)
```

---

## Usage Examples

### Configuration

**Both transmitter and receiver use the same command**:

```bash
# Enable watchdog with 20-second interval, max 3 missed packets
AT+WATCHDOG=20,3

# Disable watchdog
AT+WATCHDOG=0,0
```

### Transmitter Console Output

**Normal Operation**:
```
Watchdog enabled: 20 second interval, max 3 missed
Mode: TRANSMITTER - Monitoring receiver ACK (DO1 indicator)
...
[heartbeat sent, ACK received - no output]
[heartbeat sent, ACK received - no output]
```

**Link Lost**:
```
Watchdog: No packet for 20000 ms / expected 20000 ms (missed: 1/3)
Watchdog: No packet for 40000 ms / expected 40000 ms (missed: 2/3)
Watchdog: No packet for 60000 ms / expected 60000 ms (missed: 3/3)

=== LINK LOST ===
TX: No ACK from receiver - Setting DO1=HIGH (link lost indicator)
Waiting for link recovery (RX active)...
```

**Link Restored**:
```
=== LINK RESTORED ===
TX: DO1 indicator OFF - receiver responding
```

### Receiver Console Output

**Normal Operation**:
```
Watchdog enabled: 20 second interval, max 3 missed
Mode: RECEIVER - Monitoring transmitter heartbeat (DO2 indicator)
...
[heartbeat received - no output]
[heartbeat received - no output]
```

**Link Lost**:
```
Watchdog: No packet for 20000 ms / expected 20000 ms (missed: 1/3)
Watchdog: No packet for 40000 ms / expected 40000 ms (missed: 2/3)
Watchdog: No packet for 60000 ms / expected 60000 ms (missed: 3/3)

=== LINK LOST ===
RX: No heartbeat from transmitter - Setting RO1=LOW, RO2=LOW, DO2=HIGH
Waiting for link recovery (RX active)...
```

**Link Restored**:
```
=== LINK RESTORED ===
RX: DO2 indicator OFF - transmitter heartbeat restored
```

---

## Testing Scenarios

### Scenario 1: Receiver Powers Off

**Setup**:
- Transmitter: `AT+TDC=20000` (20s heartbeat)
- Receiver: `AT+TDC=0`
- Watchdog: `AT+WATCHDOG=20,3` (both sides)

**Steps**:
1. Both devices running normally, DO1 and DO2 off
2. Power off receiver
3. Transmitter watchdog triggers after 60s → DO1=HIGH
4. Power on receiver
5. Transmitter receives ACK → DO1 clears, link restored

**Expected Result**: ✅ Transmitter DO1 indicates receiver is offline

### Scenario 2: Transmitter Powers Off

**Setup**: Same as Scenario 1

**Steps**:
1. Both devices running normally, DO1 and DO2 off
2. Power off transmitter
3. Receiver watchdog triggers after 60s → RO1/RO2=LOW, DO2=HIGH
4. Power on transmitter
5. Receiver receives heartbeat → DO2 clears, RO outputs restored

**Expected Result**: ✅ Receiver enters safe state and DO2 indicates transmitter is offline

### Scenario 3: RF Interference

**Setup**: Same as Scenario 1

**Steps**:
1. Both devices running normally
2. Block RF signal (metal enclosure, distance, etc.)
3. Both watchdogs trigger after 60s
   - Transmitter: DO1=HIGH (no ACK received)
   - Receiver: RO1/RO2=LOW, DO2=HIGH (no heartbeat)
4. Remove RF blockage
5. Link restores automatically

**Expected Result**: ✅ Both sides indicate link lost, both recover automatically

---

## Technical Considerations

### 1. Why Same Parameters?

The heartbeat interval (TDC) is the same on both sides - if transmitter sends every 20s, receiver should expect packets every 20s. Using different watchdog intervals would create asymmetric monitoring and potential false positives.

### 2. Why Different Actions?

- **Transmitter**: Maintains last known RO/DO states during link loss
  - Reason: The transmitter's outputs may control critical systems that should maintain their last state until link is restored
  - DO1 indicator provides visual feedback without disrupting outputs

- **Receiver**: Forces RO outputs to LOW (safe state)
  - Reason: The receiver's RO outputs are typically controlling external equipment
  - Safe state (LOW/OFF) prevents equipment from operating without valid commands
  - DO2 indicator provides visual feedback

### 3. DO1 Conflict Resolution

On the transmitter, DO1 normally mirrors the receiver's RO1 state. When the watchdog sets DO1=HIGH (link lost), this overrides the mirroring behavior. When the link is restored:

1. `watchdog_reset()` clears DO1 to LOW
2. Immediately after, `RxData()` processes the feedback packet
3. DO1 is set based on receiver's RO1 (normal mirroring resumes)

This provides a clean transition from "link lost" indicator back to normal operation.

### 4. RTC-Based Timing

The watchdog uses `TimerGetCurrentTime()` (RTC-based) instead of `HAL_GetTick()`. This is critical because:
- `HAL_GetTick()` stops during low-power STOP mode
- RTC continues running in STOP mode
- Ensures accurate timing even when MCU enters low-power states

---

## Benefits

1. ✅ **Unified Configuration**: Single `AT+WATCHDOG` command works for both TX and RX
2. ✅ **Auto-Detection**: No manual mode configuration needed
3. ✅ **Bidirectional Monitoring**: Both sides know when the other is offline
4. ✅ **Visual Indicators**: DO1 (TX) and DO2 (RX) provide at-a-glance link status
5. ✅ **Automatic Recovery**: Link restores seamlessly when communication resumes
6. ✅ **Safety**: Receiver enters safe state (RO outputs LOW) when transmitter fails
7. ✅ **No Code Duplication**: Reuses existing watchdog structure and timer
8. ✅ **Backward Compatible**: v1.6.0 receiver-only watchdog behavior preserved

---

## File Changes Summary

| File | Lines Changed | Description |
|------|---------------|-------------|
| `main.c` | ~70 lines | Mode detection, DO1 handling, documentation |
| `README.md` | ~75 lines | New "Bidirectional Watchdog" section, updated version history |

**Total**: ~145 lines added/modified

---

## Comparison: v1.6.0 vs v1.6.1

| Feature | v1.6.0 | v1.6.1 |
|---------|--------|--------|
| **Receiver Watchdog** | ✅ Yes | ✅ Yes (unchanged) |
| **Transmitter Watchdog** | ❌ No | ✅ Yes (NEW) |
| **DO1 Link Indicator (TX)** | ❌ No | ✅ Yes (NEW) |
| **DO2 Link Indicator (RX)** | ✅ Yes | ✅ Yes |
| **Safe State (RX)** | ✅ Yes | ✅ Yes |
| **Auto-Detection** | ❌ No (RX only) | ✅ Yes (NEW) |
| **Configuration** | `AT+WATCHDOG` | `AT+WATCHDOG` (same) |
| **Separate Watchdog** | N/A | ❌ No (reuses structure) |

---

## Future Enhancements (Optional)

1. **Configurable Actions**: Allow user to configure what happens on link lost
   - Example: `AT+WDGACTION=1,<action>` (TX action), `AT+WDGACTION=0,<action>` (RX action)
   
2. **Different Intervals**: Allow TX and RX to have different watchdog intervals
   - Trade-off: More complex configuration, less synchronized

3. **Link Quality Metrics**: Track packet loss percentage, RSSI trends
   - Could predict link loss before it happens

4. **Audible Alarms**: Trigger buzzer/beeper when link is lost
   - Hardware-dependent

---

## Conclusion

Version 1.6.1 extends the watchdog system to provide **complete bidirectional link monitoring** without requiring separate watchdog structures or complex configuration. The implementation is:

- ✅ **Simple**: Same parameters for both sides
- ✅ **Automatic**: Mode detection based on TDC
- ✅ **Robust**: Handles link loss and recovery gracefully
- ✅ **Safe**: Receiver enters safe state when transmitter fails
- ✅ **Visible**: LED indicators show link status

The user's requirement for **transmitter heartbeat visualization** is fully met with **minimal code changes** and **no additional complexity**.

