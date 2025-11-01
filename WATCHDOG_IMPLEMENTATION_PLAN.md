# Rock-Solid Watchdog Implementation Plan - v1.6.0

**Target**: Receiver-side link monitoring with automatic safe state and recovery

---

## Current Implementation Status

### ✅ Already Implemented:
- watchdog_t structure with all necessary fields
- watchdog_init() - Initialization function
- watchdog_reset() - Reset on packet reception
- watchdog_trigger_safe_state() - Sets all RO/DO to LOW
- watchdog_timer_callback() - Periodic checking
- Safe state detection and logging

### ❌ Missing/Needs Enhancement:

1. **AT Command for Configuration**
   - No AT+WATCHDOG command exists
   - Need enable/disable control
   - Need timeout configuration
   - Need flash persistence

2. **Recovery Mechanism**
   - Safe state trigger exists, but recovery is incomplete
   - Need to restore outputs based on last known transmitter state
   - Need to handle case where transmitter state changed during outage

3. **Output Timer Management**
   - DooutputONETimer, DooutputTWOTimer, RelayONETimer, RelayTWOTimer not stopped in safe state
   - Could cause outputs to change after entering safe state

4. **Integration with Packet Reception**
   - watchdog_receive_ping() is called but needs verification
   - Should work with both heartbeat and DI-triggered packets

5. **Visual/Audible Indication**
   - No LED indication of safe state
   - Could add blinking pattern for easier debugging

6. **Edge Case Handling**
   - What if safe state triggers during TX?
   - What if recovery packet has different state than before outage?
   - What if multiple recovery attempts fail?

---

## Rock-Solid Design Enhancements

### 1. AT Command Implementation

**Command**: `AT+WATCHDOG=<enable>,<timeout_seconds>`

**Examples:**
```
AT+WATCHDOG=1,50     # Enable with 50-second timeout
AT+WATCHDOG=0        # Disable watchdog
AT+WATCHDOG?         # Query current settings
```

**Behavior:**
- Enable/disable watchdog monitoring
- Set timeout in seconds (minimum 10s, maximum 300s)
- Save to flash for persistence across resets
- Return current configuration on query

---

### 2. Enhanced Safe State Trigger

**Current Issues:**
- ✅ Sets all outputs LOW
- ❌ Doesn't stop output timers (could change outputs later)
- ❌ Doesn't save state before entering safe mode
- ❌ No LED indication

**Improvements:**
```c
static void watchdog_trigger_safe_state(void)
{
    PPRINTF("\r\n!!! WATCHDOG TIMEOUT !!!\r\n");
    
    // 1. Stop all output timers to prevent changes during safe state
    TimerStop(&DooutputONETimer);
    TimerStop(&DooutputTWOTimer);
    TimerStop(&RelayONETimer);
    TimerStop(&RelayTWOTimer);
    
    // 2. Save current output states (for potential recovery comparison)
    uint8_t saved_DO1 = DO1_flag;
    uint8_t saved_DO2 = DO2_flag;
    uint8_t saved_RO1 = RO1_flag;
    uint8_t saved_RO2 = RO2_flag;
    
    // 3. Set all outputs to SAFE STATE (LOW)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);  // DO1 = LOW
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);  // DO2 = LOW
    HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO1_PIN, GPIO_PIN_RESET);  // RO1 = LOW
    HAL_GPIO_WritePin(Relay_GPIO_PORT, Relay_RO2_PIN, GPIO_PIN_RESET);  // RO2 = LOW
    
    // 4. Update flags
    DO1_flag = 0;
    DO2_flag = 0;
    RO1_flag = 0;
    RO2_flag = 0;
    
    // 5. Set safe state indicator
    watchdog.safe_state_active = true;
    watchdog.link_active = false;
    
    // 6. LED indication (blink pattern or solid on)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);  // LED ON = safe state
    
    // 7. Log what was changed
    PPRINTF("Previous state: DO1=%d, DO2=%d, RO1=%d, RO2=%d\r\n",
            saved_DO1, saved_DO2, saved_RO1, saved_RO2);
    PPRINTF("Safe state active - Waiting for link restoration...\r\n");
}
```

---

### 3. Smart Recovery Function

**Current Issues:**
- ✅ Detects recovery
- ❌ Doesn't restore outputs intelligently
- ❌ Doesn't handle state changes during outage

**Improved Recovery Logic:**
```c
static void watchdog_recover_from_safe_state(void)
{
    PPRINTF("\r\n=== WATCHDOG: Link Restored! ===\r\n");
    
    // 1. Clear safe state flag
    watchdog.safe_state_active = false;
    watchdog.link_active = true;
    watchdog.missed_count = 0;
    
    // 2. Turn off safe state LED indicator
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
    
    // 3. DON'T restore outputs immediately!
    //    Wait for next packet from transmitter to get CURRENT state
    //    The packet will update outputs via RxData() processing
    
    PPRINTF("Outputs will be updated when next packet is processed\r\n");
    PPRINTF("Link monitoring resumed\r\n");
}
```

**Why not restore immediately?**
- Transmitter's state might have changed during outage
- Next packet contains the TRUE current state
- Safer to wait for confirmed state than guess

---

### 4. Watchdog Integration Points

**Where watchdog_reset() should be called:**

1. ✅ **test_OnRxDone()** - Already called via watchdog_receive_ping()
2. **RxData() case 0x01** - After processing request packet
3. **After successful feedback TX** - Confirms bidirectional link

**Verification needed:**
```c
static void test_OnRxDone(...)
{
    watchdog_receive_ping();  // ← Already there
    ...
}

static void RxData(...)
{
    // After processing request packet (case 0x01)
    watchdog_reset();  // ← ADD THIS
    ...
}
```

---

### 5. Output Timer Management

**Critical Issue**: Output timers could fire during safe state!

**Solution**: Stop ALL timers when entering safe state, restart them on recovery

```c
// In watchdog_trigger_safe_state():
TimerStop(&DooutputONETimer);
TimerStop(&DooutputTWOTimer);
TimerStop(&RelayONETimer);
TimerStop(&RelayTWOTimer);

// Note: Timers will restart naturally when next packet is processed
```

---

### 6. Edge Cases to Handle

#### Case 1: Safe State During TX
**Scenario**: Watchdog timeout while receiver is sending feedback

**Solution**:
```c
// In watchdog_trigger_safe_state():
if(sending_flag == 1)
{
    PPRINTF("Watchdog: Waiting for TX to complete before safe state\r\n");
    // Set a flag to trigger safe state after TX complete
    watchdog_safe_state_pending = true;
    return;
}

// In test_OnTxDone():
if(watchdog_safe_state_pending)
{
    watchdog_safe_state_pending = false;
    watchdog_trigger_safe_state();
}
```

#### Case 2: Transmitter State Changed During Outage
**Scenario**: Link lost, transmitter's DI changed, link restored

**Solution**:
- On recovery, receiver processes first packet
- Packet contains CURRENT DI states from transmitter
- Receiver updates outputs based on current state (not old state)
- **Natural recovery without assumptions**

#### Case 3: Partial Link (Some Packets Received)
**Scenario**: 50% packet loss, intermittent reception

**Solution**:
- Already handled by `max_missed` counter
- Watchdog only triggers after consecutive misses
- Single successful packet resets counter
- Prevents false alarms from temporary interference

#### Case 4: Recovery Packet Lost
**Scenario**: Link restores but first recovery packet is lost

**Solution**:
- Next packet (heartbeat or DI-triggered) will be received
- Recovery happens on ANY valid packet
- No special "recovery packet" needed

---

### 7. AT Command Specification

#### AT+WATCHDOG=<enable>,<timeout>

**Parameters:**
- `enable`: 0 (disable) or 1 (enable)
- `timeout`: Timeout in seconds (10-300)

**Examples:**
```
AT+WATCHDOG=1,50
Response: OK
         Watchdog enabled: 50 second timeout
         
AT+WATCHDOG=0
Response: OK
         Watchdog disabled
         
AT+WATCHDOG?
Response: 1,50
         (enabled, 50 second timeout)
```

**Flash Storage:**
- Save watchdog.enabled
- Save watchdog.interval_seconds
- Save watchdog.max_missed (default: 3)
- Load on boot

---

### 8. Implementation Checklist

#### Phase 1: Core Functionality (Already Mostly Done)
- [x] watchdog_t structure
- [x] watchdog_init()
- [x] watchdog_reset()
- [x] watchdog_trigger_safe_state()
- [x] watchdog_timer_callback()
- [ ] Stop output timers in safe state ← **ADD THIS**
- [ ] LED indication of safe state ← **ADD THIS**

#### Phase 2: Recovery Enhancement
- [x] Detect recovery (safe_state_active flag)
- [x] Reset missed counter
- [ ] Recovery function (separate from reset) ← **ENHANCE**
- [ ] Handle state during recovery ← **VERIFY**

#### Phase 3: AT Command
- [ ] AT+WATCHDOG command definition ← **ADD**
- [ ] at_WATCHDOG_set() implementation ← **ADD**
- [ ] at_WATCHDOG_get() implementation ← **ADD**
- [ ] Command registration ← **ADD**
- [ ] Flash persistence ← **ADD**

#### Phase 4: Integration
- [x] Call watchdog_reset() in test_OnRxDone()
- [ ] Call watchdog_reset() in RxData() after processing ← **ADD**
- [ ] Handle safe state during TX ← **ADD**
- [ ] Visual indication (LED blinking) ← **ADD**

#### Phase 5: Testing
- [ ] Test normal operation with watchdog enabled
- [ ] Test safe state trigger after timeout
- [ ] Test recovery after link restoration
- [ ] Test rapid toggling during recovery
- [ ] Test safe state during TX
- [ ] Test flash persistence across resets

---

## Recommended Implementation Order

### Step 1: Stop Timers in Safe State (Critical)
Add timer stops to `watchdog_trigger_safe_state()` to prevent outputs from changing during safe state.

### Step 2: Add watchdog_reset() Calls
Ensure watchdog is reset in all packet reception paths (RxData case 0x01).

### Step 3: Implement AT+WATCHDOG Command
Full configuration interface with flash persistence.

### Step 4: Add LED Indication
Visual feedback for safe state (easier debugging).

### Step 5: Handle Safe State During TX
Defer safe state trigger if TX in progress.

### Step 6: Enhanced Recovery
Separate recovery function with intelligent state handling.

---

## Additional Considerations

### 1. Watchdog on Transmitter Side?
**Question**: Should transmitter also have watchdog to detect lost receiver?

**Answer**: Generally NO for point-to-point with transmitter as master:
- Transmitter sends regardless of receiver
- Transmitter's outputs don't depend on receiver (it mirrors receiver's outputs)
- If you want bidirectional monitoring, implement on both sides

### 2. Heartbeat Adjustment During Outage?
**Question**: Should receiver increase heartbeat request rate during safe state?

**Answer**: NO - Receiver doesn't control transmitter's heartbeat
- Transmitter sends at its own TDC interval
- Receiver just waits and monitors

### 3. Safe State on Partial Link Loss?
**Question**: Should safe state trigger on packet errors (not just timeouts)?

**Answer**: NO - Use timeout-based approach:
- Occasional packet errors are normal (RF interference)
- Only sustained loss (multiple consecutive timeouts) triggers safe state
- Prevents false alarms

### 4. Different Safe States for Different Outputs?
**Question**: Should DO1, DO2, RO1, RO2 have configurable safe states?

**Answer**: FUTURE ENHANCEMENT:
- v1.6.0: All outputs → LOW (simple, safe)
- Future: AT+WATCHDOG_SAFESTATE=<DO1>,<DO2>,<RO1>,<RO2>
- Allows custom safe states (e.g., RO1=HIGH for fail-safe relay)

---

## Summary: What Needs to be Added

### Critical (Must Have):
1. ✅ Stop output timers in safe state
2. ✅ AT+WATCHDOG command (enable, timeout)
3. ✅ Flash persistence for watchdog config
4. ✅ Call watchdog_reset() in all RX paths

### Important (Should Have):
5. ✅ LED indication of safe state
6. ✅ Handle safe state during TX
7. ✅ Enhanced logging for debugging

### Nice to Have (Optional):
8. ⭕ Blinking LED pattern (not solid)
9. ⭕ Configurable safe states per output
10. ⭕ Watchdog statistics (total timeouts, recoveries)

---

**Shall I implement the Critical and Important items for v1.6.0?**

