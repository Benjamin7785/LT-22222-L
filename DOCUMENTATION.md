# LT-22222-L Point to Point Communication - Enhanced v1.6.0

**Enhanced LoRa P2P firmware with advanced reliability features**

---

## Table of Contents

1. [Overview](#1-overview)
2. [Enhanced Features in v1.6.0](#2-enhanced-features-in-v160)
   - [2.1 DI State Change Queue](#21-di-state-change-queue)
   - [2.2 Watchdog Link Monitoring](#22-watchdog-link-monitoring)
   - [2.3 RO/DO Decoupling](#23-rodo-decoupling)
   - [2.4 Improved State Machine](#24-improved-state-machine)
3. [Communication Modes](#3-communication-modes)
   - [3.1 Point-to-Point](#31-point-to-point)
   - [3.2 Point-to-Multipoint](#32-point-to-multipoint)
4. [AT Commands](#4-at-commands)
   - [4.1 Standard Commands](#41-standard-commands)
   - [4.2 Enhanced Commands (v1.6.0)](#42-enhanced-commands-v160)
   - [4.3 Trigger Configuration](#43-trigger-configuration)
   - [4.4 RO/DO Configuration](#44-rodo-configuration)
   - [4.5 Power Management](#45-power-management)
5. [Data Format](#5-data-format)
6. [Quick Start Guides](#6-quick-start-guides)
   - [6.1 Transmitter Setup](#61-transmitter-setup)
   - [6.2 Receiver Setup](#62-receiver-setup)
   - [6.3 Receiver with Watchdog](#63-receiver-with-watchdog)
7. [Troubleshooting](#7-troubleshooting)
8. [Firmware Files](#8-firmware-files)

---

## 1. Overview

This enhanced firmware for the Dragino LT-22222-L provides robust Point-to-Point (P2P) LoRa communication with advanced reliability features. Based on the original Dragino P2P protocol, **version 1.6.0** introduces significant improvements for industrial applications requiring high reliability.

**Repository**: [GitHub - LT-22222-L Enhanced](https://github.com/Benjamin7785/LT-22222-L)

### Key Improvements Over Stock Firmware

- ✅ **DI State Change Queue**: Never miss digital input changes during heartbeat transmissions
- ✅ **Watchdog Link Monitoring**: Automatic safe state when communication is lost
- ✅ **RO/DO Decoupling**: Independent control of relay outputs and digital outputs on receiver
- ✅ **Event-Driven Protocol**: Reliable acknowledgement without static delays
- ✅ **ISR-Level Capture**: Accurate state tracking even during rapid toggling
- ✅ **Enhanced Error Recovery**: Robust fallback mechanisms for radio failures
- ✅ **Clean Console Output**: Professional logging for debugging

---

## 2. Enhanced Features in v1.6.0

### 2.1 DI State Change Queue

**Problem Solved**: In previous versions, if DI1 or DI2 changed state during a heartbeat transmission, the change would be lost or delayed until the next heartbeat cycle.

**Solution**: Intelligent queuing system that captures DI state changes at the ISR (interrupt) level and immediately transmits them after heartbeat acknowledgement is received.

#### How It Works

1. **Normal Operation**: DI changes trigger immediate transmission
2. **During Heartbeat**: DI changes are queued with ISR-level state capture
3. **After Acknowledgement**: Queued changes are processed and sent immediately
4. **Multiple Toggles**: Only the latest state is transmitted (prevents redundant packets)

#### Benefits

- 🔹 Zero data loss on DI state changes
- 🔹 Sub-second response time even during heartbeats
- 🔹 Handles rapid toggling (only latest state matters)
- 🔹 Automatic heartbeat timer reset when DI activity occurs

#### Console Output Example

```
Radio busy during DI1 change - queuing state (sending_flag=1)
DI1 queued
...
Processing queued DI changes (DI1=1, DI2=0)
TX on freq 868700000 Hz at SF 12
```

---

### 2.2 Watchdog Link Monitoring

**Problem Solved**: If the transmitter fails or loses power, the receiver continues to hold the last relay output states indefinitely, which may be unsafe.

**Solution**: Configurable watchdog timer that monitors link health and enters a safe state if communication is lost.

#### Features

- **Configurable Timeout**: Set watchdog interval (e.g., 20 seconds)
- **Missed Packet Counter**: Tolerance for temporary radio interference (e.g., 3 missed packets)
- **Safe State**: All RO outputs go LOW when watchdog triggers
- **Visual Indicator**: DO2 goes HIGH to indicate safe state
- **Automatic Recovery**: Normal operation resumes when link is restored
- **RTC-Based Timer**: Accurate timing even during low-power STOP mode

#### Configuration

```
AT+WATCHDOG=1,20     // Enable watchdog: 20-second interval, 3 missed packets max
AT+WATCHDOG=0        // Disable watchdog
AT+WATCHDOG=?        // Query current settings
```

#### Example Scenario

**Transmitter**:
- Configured with `AT+TDC=20000` (20-second heartbeat)

**Receiver**:
- Configured with `AT+WATCHDOG=1,20` (20-second watchdog)
- If transmitter fails to send 3 consecutive heartbeats:
  - Watchdog triggers after 60 seconds (20s × 3)
  - RO1 → LOW, RO2 → LOW
  - DO2 → HIGH (visual indicator)
  - Console: `=== LINK LOST ===`
- When transmitter recovers:
  - Watchdog resets automatically
  - RO outputs resume normal operation
  - DO2 → LOW
  - Console: `=== LINK RESTORED ===`

#### Console Output Example

```
Watchdog: No packet for 20000 ms / expected 20000 ms (missed: 1/3)
Watchdog: No packet for 40000 ms / expected 40000 ms (missed: 2/3)
Watchdog: No packet for 60000 ms / expected 60000 ms (missed: 3/3)

=== LINK LOST ===
Setting RO1=LOW, RO2=LOW, DO2=HIGH
Waiting for link recovery (RX active)...

=== LINK RESTORED ===
DO2 indicator OFF - link recovered
```

---

### 2.3 RO/DO Decoupling

**Problem Solved**: In previous versions, both RO (relay outputs) and DO (digital outputs) were controlled by the transmitter's DI inputs, limiting flexibility.

**Solution**: On the receiver, RO and DO are now independent:
- **RO1/RO2**: Controlled by transmitter's DI1/DI2 (remote control)
- **DO1/DO2**: Local control only (receiver-side logic)
- **DO2**: Used as watchdog visual indicator in safe state

#### Benefits

- 🔹 RO outputs mirror transmitter DI states (original functionality)
- 🔹 DO outputs available for local feedback and indicators
- 🔹 DO2 provides visual watchdog status
- 🔹 No interference between remote control and local indicators

#### Receiver Console Output

```
RX Request (triggers=0)
Decoupled: DO=local, RO=remote
RX: DI1=1, DI2=0
RO1: DI1=1 → inverse
RO2: DI2=0 → inverse
RO1=0, RO2=1 → Sending ACK
```

---

### 2.4 Improved State Machine

**Enhancements**:
- Non-blocking delays (replaces `HAL_Delay`)
- Explicit radio state transitions (IDLE → TX_PREPARING → TX_ACTIVE → RX_PREPARING → RX_ACTIVE)
- Event-driven acknowledgement (no static timeouts)
- Enhanced error recovery with fallback radio parameters
- Atomic queue operations with interrupt protection

---

## 3. Communication Modes

### 3.1 Point-to-Point

One transmitter (TX) communicates with one receiver (RX).

#### Topology

```
┌─────────────┐                    ┌─────────────┐
│  Device A   │ ◄─────────────────►│  Device B   │
│ (TX/RX)     │   868.7 / 869.0    │  (RX/TX)    │
└─────────────┘      MHz           └─────────────┘
```

#### Configuration Example

**Device A (Transmitter)**:
```
AT+GROUPMOD=0          // Point-to-point mode
AT+TXCHS=868700000     // TX frequency: 868.7 MHz
AT+RXCHS=869000000     // RX frequency: 869.0 MHz
AT+TDC=20000           // Heartbeat every 20 seconds
AT+TRIG1=2,50          // DI1 trigger: both edges, 50ms debounce
AT+TRIG2=2,50          // DI2 trigger: both edges, 50ms debounce
AT+DI1TORO1=2          // DI1 HIGH → RO1 CLOSE on receiver
AT+DI2TORO2=2          // DI2 HIGH → RO2 CLOSE on receiver
AT+PRESETTX            // Apply quick preset
```

**Device B (Receiver)**:
```
AT+GROUPMOD=0          // Point-to-point mode
AT+TXCHS=869000000     // TX frequency: 869.0 MHz (swapped)
AT+RXCHS=868700000     // RX frequency: 868.7 MHz (swapped)
AT+TDC=0               // Receiver mode (no heartbeat)
AT+WATCHDOG=1,20       // Enable watchdog: 20s interval
AT+DI1TORO1=2          // DI1 → RO1 mapping
AT+DI2TORO2=2          // DI2 → RO2 mapping
AT+PRESETRX            // Apply quick preset with watchdog
```

#### Serial Port Display

**Transmitter**:
```
***** UpLinkCounter= 5 *****
TX on freq 868700000 Hz at SF 12
TX: Request
Flags: request=1, accept=0
TX complete, returning to RX mode
txDone
RX on freq 869000000 Hz at SF 12
rxWaiting
rxDone
Data: 00 00 01 11 00 00 20 00 00
Received ACK Group ID, sub-id: 0
=== FEEDBACK RECEIVED ===
Remote RO1: 0 → Mirroring to local DO1
Remote RO2: 1 → Mirroring to local DO2
```

**Receiver**:
```
rxDone
Data: 00 01 00 11 00 00 20 00 00
RX Request (triggers=0)
Decoupled: DO=local, RO=remote
RX: DI1=1, DI2=0
RO1: DI1=1 → inverse
RO2: DI2=0 → inverse
RO1=0, RO2=1 → Sending ACK

***** UpLinkCounter= 1 *****
TX on freq 869000000 Hz at SF 12
TX: Heartbeat
Flags: request=0, accept=1
TX Feedback: RO1=0, RO2=1
```

---

### 3.2 Point-to-Multipoint

One transmitter (TX group) communicates with multiple receivers (RX group).

#### Topology

```
                    ┌─────────────┐
              ┌────►│  RX Group 1 │
              │     └─────────────┘
┌───────────┐ │
│ TX Group  │─┼────►┌─────────────┐
│ (ID: 0,2) │ │     │  RX Group 2 │
└───────────┘ │     └─────────────┘
              │
              └────►┌─────────────┐
                    │  RX Group 3 │
                    └─────────────┘
```

#### Configuration Example

**TX Group Configuration**:
```
AT+GROUPMOD=0,2        // Group mode: TX, 2 RX devices
AT+TXCHS=868700000     // TX frequency
AT+RXCHS=869000000     // RX frequency
AT+TRIG1=2,50          // DI1 trigger settings
AT+TRIG2=2,50          // DI2 trigger settings
AT+DI1TORO1=1          // DI1 → RO1 mapping
AT+DI2TORO2=1          // DI2 → RO2 mapping
```

**RX Group 1 Configuration**:
```
AT+GROUPMOD=1,1        // Group mode: RX, Group ID 1
AT+TXCHS=869000000     // TX frequency
AT+RXCHS=868700000     // RX frequency
AT+TDC=0               // Receiver mode
AT+WATCHDOG=1,30       // 30-second watchdog
```

**RX Group 2 Configuration**:
```
AT+GROUPMOD=1,2        // Group mode: RX, Group ID 2
AT+TXCHS=869000000     // TX frequency
AT+RXCHS=868700000     // RX frequency
AT+TDC=0               // Receiver mode
AT+WATCHDOG=1,30       // 30-second watchdog
```

#### Behavior

- TX group broadcasts to all RX groups
- Each RX responds with ACK including group ID
- TX retransmits up to 4 times (30-second intervals) if missing ACKs
- UplinkCounter only increments when all ACKs received

---

## 4. AT Commands

### 4.1 Standard Commands

| Command | Description | Example |
|---------|-------------|---------|
| `ATZ` | Reset MCU | `ATZ` |
| `AT+FDR` | Factory reset (keys preserved) | `AT+FDR` |
| `AT+VER` | Get firmware version | `AT+VER` → `v1.6.0` |
| `AT+TXP` | TX power (max 20dBm) | `AT+TXP=14` |
| `AT+SYNC` | Sync word [0:0x12, 1:0x34] | `AT+SYNC=1` |
| `AT+PMB` | Preamble length | `AT+PMB=8` |
| `AT+TXCHS` | TX frequency (Hz) | `AT+TXCHS=868700000` |
| `AT+TXSF` | TX spreading factor (7-12) | `AT+TXSF=12` |
| `AT+RXCHS` | RX frequency (Hz) | `AT+RXCHS=869000000` |
| `AT+RXSF` | RX spreading factor (7-12) | `AT+RXSF=12` |
| `AT+BW` | Bandwidth [0:125k, 1:250k, 2:500k] | `AT+BW=0` |
| `AT+CR` | Coding rate [1:4/5, 2:4/6, 3:4/7, 4:4/8] | `AT+CR=1` |
| `AT+TDC` | Heartbeat interval (ms), 0=RX only | `AT+TDC=20000` |
| `AT+SEND` | Send custom hex data | `AT+SEND=01020304` |
| `AT+GROUPMOD` | Group mode [0:P2P, 1:RX group] | `AT+GROUPMOD=0` |
| `AT+GROUPID` | Group password (8 chars) | `AT+GROUPID=12345678` |

---

### 4.2 Enhanced Commands (v1.6.0)

#### AT+PRESETTX

**Quick preset for transmitter configuration**

```
AT+PRESETTX
```

**Configures**:
- `GROUPMOD=0` (Point-to-point)
- `TXCHS=868700000` (868.7 MHz)
- `RXCHS=869000000` (869.0 MHz)
- `TDC=20000` (20-second heartbeat)
- `TRIG1=2,50` (Both edges, 50ms debounce)
- `TRIG2=2,50` (Both edges, 50ms debounce)
- `DI1TORO1=2` (Direct mapping)
- `DI2TORO2=2` (Direct mapping)

**Response**: `Transmitter preset applied`

---

#### AT+PRESETRX

**Quick preset for receiver configuration with watchdog**

```
AT+PRESETRX
```

**Configures**:
- `GROUPMOD=0` (Point-to-point)
- `TXCHS=869000000` (869.0 MHz - swapped)
- `RXCHS=868700000` (868.7 MHz - swapped)
- `TDC=0` (Receiver mode)
- `TRIG1=2,50` (Both edges, 50ms debounce)
- `TRIG2=2,50` (Both edges, 50ms debounce)
- `DI1TORO1=2` (Direct mapping)
- `DI2TORO2=2` (Direct mapping)
- `WATCHDOG=1,50` (Enabled: 50s interval, 3 missed max)

**Response**: `Receiver preset applied (Watchdog: 50s)`

---

#### AT+WATCHDOG

**Configure watchdog link monitoring**

**Set Watchdog**:
```
AT+WATCHDOG=<enable>,<interval_seconds>

enable: 0=disable, 1=enable
interval_seconds: Expected packet interval (e.g., 20)
```

**Examples**:
```
AT+WATCHDOG=1,20     // Enable: 20s interval, 3 missed packets allowed (60s total)
AT+WATCHDOG=0        // Disable watchdog
```

**Query Watchdog**:
```
AT+WATCHDOG=?
```

**Response**:
```
1,20
Watchdog: ENABLED
Interval: 20 seconds
Max missed: 3 (60 seconds total)
```

**Behavior**:
- Watchdog timer resets on every received packet from transmitter
- After 3 missed packets (3 × interval), enters safe state:
  - RO1 → LOW
  - RO2 → LOW
  - DO2 → HIGH (visual indicator)
- Automatically recovers when valid packet received
- Uses RTC-based timer (accurate during low-power modes)

---

### 4.3 Trigger Configuration

#### AT+TRIG1 / AT+TRIG2

**Configure DI interrupt trigger mode**

```
AT+TRIG1=<mode>,<debounce>
AT+TRIG2=<mode>,<debounce>

mode: 0=falling, 1=rising, 2=both edges
debounce: Debounce time in milliseconds
```

**Examples**:
```
AT+TRIG1=2,50        // DI1: Both edges, 50ms debounce
AT+TRIG2=1,100       // DI2: Rising edge only, 100ms debounce
```

**Special Features (v1.6.0)**:
- Triggers during heartbeat TX/RX are queued automatically
- ISR-level state capture ensures accuracy
- Heartbeat timer resets on DI activity (prevents conflicts)

---

### 4.4 RO/DO Configuration

#### Mapping Commands

```
AT+DI1TODO1=<map>[,<latch_time>,<timeout_state>]
AT+DI1TORO1=<map>[,<latch_time>,<timeout_state>]
AT+DI2TODO2=<map>[,<latch_time>,<timeout_state>]
AT+DI2TORO2=<map>[,<latch_time>,<timeout_state>]
```

#### Map Values

| Value | DIx → DOx | DIx → ROx |
|-------|-----------|-----------|
| `0` | No action | No action |
| `1` | DI HIGH → DO LOW<br>DI LOW → DO HIGH | DI HIGH → RO CLOSE<br>DI LOW → RO OPEN |
| `2` | DI HIGH → DO HIGH<br>DI LOW → DO LOW | DI HIGH → RO CLOSE<br>DI LOW → RO OPEN |
| `3` | Toggle DO | Toggle RO |

#### Latch Time (Optional)

If specified, output returns to original state after timeout.

**Example**:
```
AT+DI1TORO1=2,60,1   // DI1 → RO1, hold 60s, then RO1 → OPEN
```

#### Timeout State (Optional)

| Value | Behavior |
|-------|----------|
| `0` | No change after timeout |
| `1` | DO → LOW or RO → OPEN |
| `2` | DO → HIGH or RO → CLOSE |
| `3` | Return to state before trigger |

#### Receiver Behavior (v1.6.0)

- **Transmitter**: `AT+DI1TORO1` must be configured to send trigger
- **Receiver**: Can override with its own `AT+DI1TORO1` setting
- **Decoupled**: RO follows remote DI, DO is local only

---

### 4.5 Power Management

#### AT+DOROSAVE

**Save/restore DO/RO state after power cycle**

```
AT+DOROSAVE=0              // Disable retention
AT+DOROSAVE=1              // Enable retention
AT+DOROSAVE=2,<DO1>,<DO2>,<RO1>,<RO2>  // Set default states
```

**Example**:
```
AT+DOROSAVE=2,0,0,0,1      // Power-on defaults: DO1=LOW, DO2=LOW, RO1=OPEN, RO2=CLOSE
```

---

#### AT+DI2SLEEP

**Enable DI2 as sleep control pin**

```
AT+DI2SLEEP=1              // Enable DI2 sleep control
AT+DI2SLEEP=0              // Disable DI2 sleep control
```

**Behavior**:
- Falling edge (500ms pulse) → Enter sleep mode
- Rising edge → Wake up
- DO/RO state retention follows `AT+DOROSAVE` settings

---

## 5. Data Format

**Packet Structure**: 8 bytes GROUPID + 9 bytes payload + 4 bytes CRC32

### Request Packet (TX → RX)

| Byte | Field | Description |
|------|-------|-------------|
| 0 | Address | 0x00=broadcast, 0x01-0x08=RX group ID |
| 1 | Request | Non-zero = mapping request |
| 2 | ACK | ACK from previous transmission |
| 3 | DI1 | High nibble: 0x1, Low nibble: DI1 level |
| 4 | DI1TODO1 | Trigger action for DO1 |
| 5 | DI1TORO1 | Trigger action for RO1 |
| 6 | DI2 | High nibble: 0x2, Low nibble: DI2 level |
| 7 | DI2TODO2 | Trigger action for DO2 |
| 8 | DI2TORO2 | Trigger action for RO2 |

**Example**: `00 01 00 11 00 00 20 00 00`
- Broadcast (0x00)
- Request flag set (0x01)
- DI1=1 (0x11), no trigger (0x00, 0x00)
- DI2=0 (0x20), no trigger (0x00, 0x00)

---

### Feedback Packet (RX → TX) - v1.6.0

| Byte | Field | Description |
|------|-------|-------------|
| 0 | Address | 0x00=broadcast, 0x01-0x08=RX group ID |
| 1 | Request | Always 0 (feedback) |
| 2 | ACK | ACK flag (0x01) |
| 3 | RO1 | High nibble: 0x1, Low nibble: **RO1 level** |
| 4 | DI1TODO1 | Echo from request |
| 5 | DI1TORO1 | Echo from request |
| 6 | RO2 | High nibble: 0x2, Low nibble: **RO2 level** |
| 7 | DI2TODO2 | Echo from request |
| 8 | DI2TORO2 | Echo from request |

**Note**: In v1.6.0, receivers send **RO states** (not DO states) in feedback packets due to RO/DO decoupling.

**Example**: `00 00 01 10 00 00 21 00 00`
- Broadcast (0x00)
- Feedback (0x00)
- ACK (0x01)
- RO1=0 (0x10)
- RO2=1 (0x21)

---

## 6. Quick Start Guides

### 6.1 Transmitter Setup

**Goal**: Configure a transmitter with 20-second heartbeat and DI triggers.

**Commands**:
```
AT+PRESETTX            // Apply preset configuration
AT+VER                 // Verify firmware version (should be v1.6.0)
```

**Or manually**:
```
AT+GROUPMOD=0
AT+TXCHS=868700000
AT+RXCHS=869000000
AT+TDC=20000
AT+TRIG1=2,50
AT+TRIG2=2,50
AT+DI1TORO1=2
AT+DI2TORO2=2
ATZ                    // Reset to apply
```

**Expected Console Output**:
```
DRAGINO LT-22222-L PINGPONG
Image Version: v1.6.0
Error recovery system initialized
TDC=20000: Heartbeat mode
TX on freq 868700000 Hz at SF 12
```

---

### 6.2 Receiver Setup

**Goal**: Configure a receiver for RX-only mode.

**Commands**:
```
AT+GROUPMOD=0
AT+TXCHS=869000000
AT+RXCHS=868700000
AT+TDC=0               // RX-only mode
AT+DI1TORO1=2
AT+DI2TORO2=2
ATZ                    // Reset to apply
```

**Expected Console Output**:
```
DRAGINO LT-22222-L PINGPONG
Image Version: v1.6.0
Error recovery system initialized
TDC=0: Starting in RX-only mode
RX on freq 868700000 Hz at SF 12
rxWaiting
```

---

### 6.3 Receiver with Watchdog

**Goal**: Configure a receiver with watchdog link monitoring.

**Commands**:
```
AT+PRESETRX            // Apply preset with watchdog (50s)
AT+VER                 // Verify firmware version
```

**Or manually**:
```
AT+GROUPMOD=0
AT+TXCHS=869000000
AT+RXCHS=868700000
AT+TDC=0
AT+WATCHDOG=1,20       // 20-second watchdog
AT+DI1TORO1=2
AT+DI2TORO2=2
ATZ
```

**Expected Console Output**:
```
DRAGINO LT-22222-L PINGPONG
Image Version: v1.6.0
Error recovery system initialized
Watchdog enabled: 20 second interval, max 3 missed
TDC=0: Starting in RX-only mode
RX on freq 868700000 Hz at SF 12
rxWaiting
```

**Watchdog Behavior**:
- Normal operation: Watchdog resets on every received packet
- Link lost: After 60 seconds (20s × 3), enters safe state
- Link restored: Automatic recovery

---

## 7. Troubleshooting

### Receiver Not Entering RX Mode

**Symptoms**: Console shows no `rxWaiting` after startup.

**Solution**:
```
AT+TDC=0              // Ensure TDC is 0 for receiver mode
ATZ                   // Reset
```

**Verify**: Console should show `TDC=0: Starting in RX-only mode`.

---

### DI Changes Not Transmitted During Heartbeat

**Symptoms**: DI toggle during heartbeat is ignored.

**Solution**: This is fixed in v1.6.0 via the DI queue system.

**Verify**: Console should show:
```
Radio busy during DI1 change - queuing state
...
Processing queued DI changes (DI1=1, DI2=0)
```

---

### Watchdog Not Triggering

**Symptoms**: Link is lost but watchdog doesn't activate safe state.

**Check**:
1. Verify watchdog is enabled: `AT+WATCHDOG=?`
2. Verify interval matches or exceeds TDC: If TDC=20s, watchdog should be ≥20s
3. Check console for watchdog messages: `Watchdog: No packet for X ms`

**Solution**:
```
AT+WATCHDOG=1,20      // Enable with appropriate interval
ATZ
```

---

### Uplink Counter Incrementing Randomly

**Symptoms**: UpLinkCounter jumps by large numbers.

**Solution**: Fixed in v1.6.0 by correcting diagnostic code.

**Note**: Retransmissions (due to missed ACKs) do **not** increment the counter.

---

### Receiver Restarting in Safe State

**Symptoms**: Device restarts when watchdog triggers.

**Solution**: Fixed in v1.6.0. The watchdog no longer interferes with IWDG or other timers.

**Verify**: Console should show safe state messages without reboot:
```
=== LINK LOST ===
Setting RO1=LOW, RO2=LOW, DO2=HIGH
Waiting for link recovery (RX active)...
```

---

## 8. Firmware Files

### Repository

**GitHub**: [https://github.com/Benjamin7785/LT-22222-L](https://github.com/Benjamin7785/LT-22222-L)  
**Branch**: `1.6.0`

### Files

| File | Description |
|------|-------------|
| `LT-22222-L_v1.6.0.hex` | **Ready-to-flash firmware** (root directory) |
| `Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/` | Source code |
| `DOCUMENTATION.md` | This documentation |
| `README.md` | Quick start and feature overview |
| `DI_QUEUE_IMPLEMENTATION.md` | Technical details of DI queue system |
| `WATCHDOG_IMPLEMENTATION_PLAN.md` | Watchdog design document |
| `CODE_REVIEW_FINDINGS.md` | Code review and fixes log |

### Flashing Firmware

1. Download `LT-22222-L_v1.6.0.hex` from GitHub
2. Use STM32 ST-LINK Utility or Keil µVision
3. Connect ST-LINK programmer to LT-22222-L
4. Flash hex file to device
5. Reset device and verify version: `AT+VER` → `v1.6.0`

---

## Credits

**Original Firmware**: [Dragino Technology](https://wiki.dragino.com/xwiki/bin/view/Main/%20Point%20to%20Point%20Communication%20of%20LT-22222-L/)  
**Enhanced Version**: Community-developed enhancements for industrial reliability

**License**: Revised BSD License (see source files)

---

## Changelog

### v1.6.0 (2025-11-01)

**Features**:
- ✅ DI State Change Queue with ISR-level capture
- ✅ Watchdog link monitoring with safe state
- ✅ RO/DO decoupling on receiver
- ✅ New AT commands: `AT+PRESETTX`, `AT+PRESETRX`, `AT+WATCHDOG`
- ✅ Event-driven acknowledgement protocol
- ✅ RTC-based timing for low-power accuracy

**Fixes**:
- ✅ TDC=0 now properly supported for receiver mode
- ✅ DI state changes during heartbeat no longer lost
- ✅ Watchdog timer accuracy during STOP mode
- ✅ Radio state machine race conditions
- ✅ Console output cleanup

**Performance**:
- ✅ Sub-second DI response time (even during heartbeats)
- ✅ Zero data loss on DI state changes
- ✅ Reliable acknowledgement without static delays

---

**For support and issues**: [GitHub Issues](https://github.com/Benjamin7785/LT-22222-L/issues)

**Original Documentation**: [Dragino Wiki](https://wiki.dragino.com/xwiki/bin/view/Main/%20Point%20to%20Point%20Communication%20of%20LT-22222-L/)

