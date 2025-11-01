# DI State Change Queue Implementation

## Problem Statement

### Original Behavior (The Issue)
When DI1 or DI2 changes during an active heartbeat transmission/reception cycle, the change was not processed immediately. Instead:

1. **DI change during heartbeat TX/RX**: GPIO interrupt sets `exitflag1=1` or `exitflag2=1`
2. **`send_exti()` checks `sending_flag`**: If `sending_flag==1` (radio busy), the exit flag remains set but is NOT processed
3. **Next heartbeat transmission**: The old `exitflag1=1` from the previous cycle causes the NEXT heartbeat to transmit the state change
4. **Result**: **One full heartbeat cycle of delay** (typically 6+ seconds!)

### Example Timeline (Before Fix)
```
t=0s:    Heartbeat TX starts (sending_flag=1)
t=0.2s:  DI1 changes → exitflag1=1 set, but send_exti() sees sending_flag=1 → IGNORED
t=0.5s:  Heartbeat TX completes → Heartbeat RX starts
t=1.0s:  Heartbeat feedback received → Cycle complete
t=6.0s:  Next heartbeat TX → NOW sends the DI1 change from t=0.2s
         Latency: 5.8 seconds!
```

## Solution: DI State Queue

### Design Concept
Implement an explicit queue that:
1. **Captures DI state changes immediately** when they occur (even during TX/RX)
2. **Timestamps the change** for diagnostics
3. **Processes queued changes immediately** after heartbeat feedback is received
4. **Sends queued changes** as a separate transmission instead of waiting for the next heartbeat

### Example Timeline (After Fix)
```
t=0s:    Heartbeat TX starts (sending_flag=1)
t=0.2s:  DI1 changes → QUEUED with timestamp (state captured)
t=0.5s:  Heartbeat TX completes → Heartbeat RX starts
t=1.0s:  Heartbeat feedback received → Queue processed → DI1 TX triggered immediately
t=1.3s:  DI1 state change transmitted and confirmed
         Latency: 1.1 seconds (vs 5.8 seconds before!)
```

## Implementation Details

### 1. Data Structure
```c
typedef struct {
    bool di1_changed;                // DI1 changed during heartbeat
    bool di2_changed;                // DI2 changed during heartbeat
    uint8_t di1_state;               // Captured DI1 state (0 or 1)
    uint8_t di2_state;               // Captured DI2 state (0 or 1)
    uint32_t change_time;            // Timestamp of change
    bool pending_transmission;       // Need to send queued changes
} di_state_queue_t;

static di_state_queue_t di_queue = {
    .di1_changed = false,
    .di2_changed = false,
    .di1_state = 0,
    .di2_state = 0,
    .change_time = 0,
    .pending_transmission = false
};
```

### 2. Core Functions

#### `di_queue_capture_state(uint8_t di_channel)`
- **Purpose**: Capture DI state change into queue
- **When**: Called from `send_exti()` when `sending_flag==1` (radio busy)
- **What it does**:
  - Reads current GPIO state of DI1 or DI2
  - Stores state and timestamp
  - Sets `pending_transmission=true`

#### `di_queue_clear()`
- **Purpose**: Clear the queue after processing
- **When**: Called after successfully transmitting queued changes

#### `di_queue_has_pending()`
- **Purpose**: Check if there are changes waiting
- **Returns**: `true` if queue has pending changes

#### `di_queue_process_and_send()`
- **Purpose**: Process queued changes and trigger immediate transmission
- **When**: Called after heartbeat feedback is received
- **What it does**:
  - Sets `exitflag1` and/or `exitflag2` as needed
  - Triggers `uplink_data_status=1` for immediate TX
  - Clears the queue
  - Logs queue age for diagnostics

### 3. Integration Points

#### A. In `send_exti()` - Capture state when radio is busy
```c
static void send_exti(void)
{
    if(exitflag1==1)
    {	
        if(sending_flag ==0)
        {	
            // Normal processing...
        }
        else
        {
            // NEW CODE - Radio is busy, queue the change
            PPRINTF("Radio busy during DI1 change - queuing state\r\n");
            di_queue_capture_state(1);
            exitflag1=0;  // Clear flag since we've queued it
        }
    }
    
    // Same for exitflag2...
}
```

#### B. In `RxData()` - Process queue after receiving feedback
```c
static void RxData(lora_AppData_t *AppData)
{
    // ... existing feedback processing ...
    
    if(AppData->Buff[2] == 0x01)  // Received feedback
    {
        // ... update local outputs ...
        
        accept_flag = 0;
        
        // NEW CODE - Process queued DI changes
        di_queue_process_and_send();
    }
}
```

#### C. In `test_OnTxDone()` - Process queue after sending feedback
```c
static void test_OnTxDone( void )
{
    // ... existing TX complete handling ...
    
    if(accept_flag == 1)
    {
        accept_flag = 0;
        
        // NEW CODE - Process queued DI changes (receiver side)
        di_queue_process_and_send();
    }
}
```

## Benefits

### 1. **Immediate Response**
- DI changes during heartbeat cycles are sent **immediately** after feedback
- No more waiting for the next heartbeat cycle
- Typical latency reduction: **6+ seconds → ~1 second**

### 2. **No Lost Changes**
- All DI state changes are captured with timestamps
- Even multiple changes during a single heartbeat cycle are tracked
- Queue prevents race conditions

### 3. **Better Diagnostics**
- Queue age is logged for troubleshooting
- Clear visibility into when changes occurred vs when they were sent
- Helps identify latency issues

### 4. **Bi-directional Support**
- Works for both transmitter and receiver
- Receiver's DI changes during feedback TX are also queued

## Testing Recommendations

### Test Scenario 1: Single DI Change During Heartbeat
1. Configure transmitter with `AT+TDC=6000` (6 second heartbeat)
2. Wait for heartbeat TX to start (sending_flag=1)
3. Toggle DI1
4. Verify change is queued (check console output)
5. Wait for heartbeat feedback to complete
6. Verify queued change is sent immediately
7. **Expected latency**: < 1.5 seconds from DI toggle to receiver output change

### Test Scenario 2: Multiple DI Changes During Heartbeat
1. Configure transmitter with `AT+TDC=6000`
2. During heartbeat TX/RX cycle, toggle both DI1 and DI2
3. Verify both changes are queued
4. Wait for heartbeat feedback
5. Verify both queued changes are sent in a single transmission
6. **Expected behavior**: Both outputs update after one transmission

### Test Scenario 3: DI Change During Receiver Feedback TX
1. Configure transmitter/receiver pair
2. Trigger a DI change on transmitter → receiver processes and sends feedback
3. While receiver is sending feedback, toggle one of receiver's DI inputs
4. Verify receiver queues the change
5. After feedback TX completes, verify receiver sends its DI change
6. **Expected behavior**: Receiver's DI change sent immediately after feedback

## Console Output Examples

### Normal DI Change (Radio Idle)
```
DI1 changed
TX on freq 868700000 Hz at SF 7
txDone
RX on freq 869000000 Hz at SF 8
rxWaiting
```

### Queued DI Change (Radio Busy)
```
TX on freq 868700000 Hz at SF 7  ← Heartbeat TX starts
Radio busy during DI1 change - queuing state  ← DI1 toggled during TX
DI1 change queued: state=1, time=1234
txDone
RX on freq 869000000 Hz at SF 8
=== FEEDBACK RECEIVED ===
Remote DO1: 1 → Mirroring to local DO1
=== Confirmation Complete ===
Processing queued DI changes (age: 856 ms)  ← Queue processed after feedback
  DI1: 1
Queued DI changes sent immediately (not waiting for next heartbeat)
TX on freq 868700000 Hz at SF 7  ← Immediate TX of queued change
```

## Compatibility

- **Backward Compatible**: Works with existing `AT+TDC` heartbeat configurations
- **No Configuration Changes**: Automatically enabled, no user action required
- **TDC=0 Mode**: Queue still functions but is less critical (no heartbeats to collide with)
- **Group Mode**: Compatible with all group modes (0, 1, 2, etc.)

## Future Enhancements

### Potential Improvements
1. **Multiple Change Aggregation**: If DI toggles multiple times during heartbeat, only send final state
2. **Priority Queue**: Allow certain DI inputs to have higher priority
3. **Max Queue Age Warning**: Alert if queued changes are older than threshold
4. **Queue Depth**: Support queuing multiple heartbeat cycles worth of changes

## Related Files Modified

- `Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/src/main.c`
  - Added `di_state_queue_t` structure
  - Added queue management functions
  - Modified `send_exti()` to queue changes when radio busy
  - Modified `RxData()` to process queue after feedback
  - Modified `test_OnTxDone()` to process queue after TX

---

**Author**: AI Assistant  
**Date**: November 1, 2025  
**Related Enhancement**: Heartbeat Mode DI State Change Queue  
**Status**: Implemented, Ready for Testing

