# Event-Driven Queue Processing with Guaranteed Acknowledgement

**Date**: November 1, 2025  
**Feature**: Acknowledgement-based DI state queue processing (no timeouts!)

---

## Design Philosophy

**OLD**: Timeout-based queue processing (5-second static delay)  
**NEW**: Event-driven acknowledgement system (pure handshake protocol)

### Benefits:
- ✅ **No static delays** - Pure event-driven design
- ✅ **Guaranteed queue processing** - Relies on ACK handshake
- ✅ **Faster response** - No waiting for timeouts
- ✅ **More reliable** - Clear request/response protocol

---

## The Complete Flow

### 1. Transmitter Heartbeat (No DI Activity)

**Transmitter:**
```
OnTxTimerEvent fires (15-second timer)
↓
uplink_data_status=1
↓
Main loop: Send_TX() called
↓
Send_TX: Setting request_flag=1 (accept_flag=0)
Packet bytes 9-10: request_flag=1, accept_flag=0
↓
***** UpLinkCounter= X *****
TX Type: NORMAL (exitflag1=0, exitflag2=0, heartbeat=1)
txDone
↓
TX complete, returning to RX mode
RX on freq 869000000 Hz at SF 12
rxWaiting
```

**Receiver:**
```
rxDone
Data: 00 01 00 11 00 00 21 00 00
      ↑  ↑
      |  request_flag=1 (byte 9)
      group_mode=0 (byte 8)
↓
rece_temp = Buff[1] = 0x01 → case 0x01
↓
Received REQUEST packet (has_triggers=0)
Request processed - DO1=1, DO2=0, RO1=1, RO2=0
Sending ACK to transmitter (triggers=0)...
P2P: ACK triggered (accept_flag=1, uplink_data_status=1)
↓
Main loop: TX ACK
↓
Send_TX: Sending FEEDBACK (accept_flag=1, request_flag=0)
Packet bytes 9-10: request_flag=0, accept_flag=1
↓
***** UpLinkCounter= Y *****
txDone
↓
Feedback sent, returning to normal mode
TX complete, returning to RX mode
```

**Transmitter Receives ACK:**
```
rxDone
Data: 00 00 01 10 00 00 10 00 00
      ↑  ↑
      |  accept_flag=1 (byte 10)
      request_flag=0 (byte 9)
↓
rece_temp = Buff[1] = 0x00 → case 0x00
↓
DEBUG: Before feedback processing - queue pending: 0
=== FEEDBACK RECEIVED ===
...
No queued DI changes to process  (no DI activity during heartbeat)
```

---

### 2. DI Change During Heartbeat TX/RX

**Transmitter (DI1 toggled during TX):**
```
***** UpLinkCounter= X *****
TX on freq 868700000 Hz at SF 12
↓
Radio busy during DI1 change - queuing state (sending_flag=1)
DI1 change queued: state=1, time=XXXXX
QUEUE STATUS: pending=1, di1_changed=1, di2_changed=0
DI1 queued - will be sent after feedback cycle
↓
txDone
TX complete, returning to RX mode
RX on freq 869000000 Hz at SF 12
rxWaiting
```

**Receiver:**
```
rxDone
Data: 00 01 00 11 00 00 21 00 00  (heartbeat packet, no DI change info)
↓
Received REQUEST packet (has_triggers=0)
Request processed - DO1=1, DO2=0, RO1=1, RO2=0
Sending ACK to transmitter (triggers=0)...
P2P: ACK triggered (accept_flag=1, uplink_data_status=1)
↓
***** UpLinkCounter= Y *****
(sends ACK)
txDone
```

**Transmitter Receives ACK and Processes Queue:**
```
rxDone
Data: 00 00 01 10 00 00 10 00 00
↓
DEBUG: Before feedback processing - queue pending: 1  ← Queue still there!
=== FEEDBACK RECEIVED ===
...
=== QUEUE PROCESSING ===
Queued DI changes detected (age: 1200 ms)
  DI1 changed: 1 (state: 1)
  DI2 changed: 0 (state: 0)
  Setting exitflag1=1 for queued DI1 (state=1)
Queue cleared, queued states stored for Send_TX()
↓
TDC timer reset by DI1 activity
↓
***** UpLinkCounter= X+1 *****
Using QUEUED DI1 state: 1  ← Sends the queued state!
TX Type: NORMAL (exitflag1=1, exitflag2=0, heartbeat=0)
Queued DI states used in TX, flag cleared
txDone
```

**Receiver Processes DI Change:**
```
rxDone
Data: 00 01 00 11 04 04 21 00 00  ← Now has trigger bytes!
      ↑  ↑     ↑  ↑  ↑
      |  |     |  DI1 trigger bytes (04 04)
      |  |     DI1 state (11 = HIGH)
      |  request_flag=1
      group_mode=0
↓
Received REQUEST packet (has_triggers=1)
(processes DI1 change, updates DO1)
Sending ACK to transmitter (triggers=1)...
```

---

## Key Changes Made

### 1. Request Flag Always Set (lines 626-636)
```c
if(accept_flag==0)  // Transmitter
{
    request_flag=1;  // ALWAYS set for transmitter packets
}
```

### 2. Receiver Processes ALL Requests (lines 992-999)
```c
// Process ALL request packets, not just ones with trigger bytes
bool has_triggers = ((AppData->Buff[4]!=0x00)||(AppData->Buff[5]!=0x00)||(AppData->Buff[7]!=0x00)||(AppData->Buff[8]!=0x00));
PPRINTF("Received REQUEST packet (has_triggers=%d)\r\n", has_triggers);
// Always process (whether heartbeat or DI-triggered)
```

### 3. Receiver ALWAYS Sends ACK (lines 1161-1168)
```c
// CRITICAL: ALWAYS send acknowledgement for ALL request packets
uplink_data_status=1;
PPRINTF("P2P: ACK triggered (accept_flag=%d, uplink_data_status=%d)\r\n", accept_flag, uplink_data_status);
```

### 4. Queue Processed on ACK Reception (lines 948-977)
```c
if(di_queue_has_pending())
{
    // Set exitflags with queued states
    exitflag1/2 = 1;
    queued_di1/2_state = di_queue.di1/2_state;
    di_queue_clear();
}
```

### 5. Send_TX Uses Queued States (lines 639-649, 671-681)
```c
if(use_queued_di_states && exitflag1==1)
{
    DI1_flag = queued_di1_state;  // Use QUEUED state, not current GPIO
}
```

---

## Expected Behavior After Rebuild

### Heartbeat Acknowledgement:
- ✅ Transmitter sends heartbeat with `request_flag=1`
- ✅ Receiver ALWAYS sends ACK (even if no state changes)
- ✅ Transmitter receives ACK and checks queue
- ✅ If queue empty: Normal operation continues
- ✅ If queue has changes: Processes immediately and sends DI states

### DI Change During Heartbeat:
- ✅ DI change detected → queued (not lost!)
- ✅ Heartbeat cycle completes normally
- ✅ ACK received → queue processed
- ✅ Queued DI state sent in next TX (within 1-2 seconds, not 15 seconds!)
- ✅ Receiver processes DI change and updates outputs
- ✅ Clean, event-driven flow

---

## Debug Output to Verify

**After rebuild, you should see:**

1. Every transmitter heartbeat: `"Send_TX: Setting request_flag=1"`
2. Every receiver RX: `"Received REQUEST packet (has_triggers=0)"` for heartbeats
3. Every receiver ACK: `"P2P: ACK triggered"`
4. Transmitter receives ACK: `"FEEDBACK RECEIVED"` + queue check
5. If queue has data: `"QUEUE PROCESSING"` → `"Setting exitflag1=1"`
6. Next TX: `"Using QUEUED DI1 state: X"`

**If you DON'T see these messages, the debug output will pinpoint exactly where the flow breaks.**

---

**Rebuild and test! Share the console output and we'll verify the acknowledgement system is working!** 🎯

