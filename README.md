# LT-22222-L Enhanced Firmware

## Overview
Enhanced Private LoRa Protocol firmware for DRAGINO LT-22222-L microcontrollers with improved reliability, performance, and features.

**Original Source**: http://wiki.dragino.com/xwiki/bin/view/Main/%20Point%20to%20Point%20Communication%20of%20LT-22222-L/

## Key Enhancements

### 1. **DI State Change Queue** (NEW - Nov 2025)
**Problem Solved**: DI1/DI2 changes during heartbeat TX/RX cycles were delayed by one full heartbeat cycle (6+ seconds).

**Solution**: Automatic queue that captures DI state changes during heartbeat cycles and transmits them immediately after feedback is received.

**Benefits**:
- ✅ Latency reduced from 6+ seconds to ~1 second
- ✅ No lost DI state changes
- ✅ Automatic - no configuration needed
- ✅ Works for both transmitter and receiver

**Details**: See `DI_QUEUE_IMPLEMENTATION.md` for complete documentation.

### 2. **Enhanced Error Recovery**
- Automatic fallback radio parameters on consecutive failures
- Retry logic with configurable max attempts
- Error statistics tracking and diagnostics
- Graceful recovery to primary parameters after errors clear

### 3. **Non-Blocking Delays**
- Replaced blocking `HAL_Delay()` calls with non-blocking timers
- Improved radio responsiveness during group timing
- Better handling of RX windows and feedback delays

### 4. **Bidirectional State Mirroring**
- Transmitter mirrors receiver's DO/RO states locally
- Complete feedback loop for state confirmation
- Both sides stay in sync automatically

### 5. **Simplified Configuration**
- `AT+PRESET` command for quick one-line setup
- Pre-configured asymmetric spreading factors (TX SF7, RX SF8)
- Optimized for low latency and reliable delivery

### 6. **Performance Optimizations**
- `FAST_MODE` compile flag to disable verbose debug output (~25-30ms faster)
- Reset-on-activity TDC timer (prevents heartbeat collisions with DI-triggered TX)
- Optimized TX/RX state machine transitions

### 7. **Comprehensive Testing**
- `AT+TEST` command for automated validation
- State machine validation
- Error recovery validation  
- Non-blocking delay validation
- Radio error simulation

## Quick Start

### Transmitter Setup
```
AT+PRESET=TX    # Configure as transmitter (SF7/SF8, 20dBm)
AT+TDC=6000     # Optional: 6-second heartbeat (0 to disable)
AT+DI1TODO1=1   # Map DI1 to receiver's DO1
AT+RXFREQ=869000000  # Set RX frequency
```

### Receiver Setup
```
AT+PRESET=RX    # Configure as receiver (SF8/SF7, 14dBm)
AT+TDC=0        # Disable automatic TX (receiver mode)
AT+TXFREQ=869000000  # Set TX frequency (for feedback)
```

## Advanced Features

### DI State Queue (Heartbeat Mode)
When using `AT+TDC>0` (heartbeat mode), DI changes during heartbeat cycles are automatically queued and sent immediately after feedback:

```
t=0s:    Heartbeat TX
t=0.2s:  DI1 changes → Queued (not delayed!)
t=1.0s:  Heartbeat feedback received → Queue processed → DI1 TX triggered
t=1.3s:  DI1 change confirmed at receiver
```

**No configuration needed** - the queue is always active and transparent.

### Performance Tuning
For ultra-low latency applications:
1. **Enable FAST_MODE**: Add `-DFAST_MODE` to compiler flags (saves ~25-30ms per TX)
2. **Use SF7/SF7**: Lower spreading factors for faster airtime (trade reliability for speed)
3. **Disable TDC**: Set `AT+TDC=0` for event-driven operation only

## Compilation

### Using Keil MDK-ARM
1. Open `LRWAN_LoRaWAN.uvprojx` in Keil µVision
2. Optional: Add `-DFAST_MODE` to C/C++ preprocessor defines
3. Build project (Keil Professional license recommended, >32KB code)
4. Output: `*.hex` file in project directory

### Using ARM GCC (Free, No Limits)
```bash
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb \
  -DSTM32L072xx -DUSE_HAL_DRIVER -DFAST_MODE \
  -O2 -ffunction-sections -fdata-sections \
  -I"Drivers/CMSIS/Device/ST/STM32L0xx/Include" \
  -I"Drivers/CMSIS/Include" \
  -I"Drivers/STM32L0xx_HAL_Driver/Inc" \
  [... all source files ...]
  -o firmware.elf
arm-none-eabi-objcopy -O ihex firmware.elf firmware.hex
```

## Firmware Upgrade
Upload the generated `.hex` file using STM32 Cube Programmer or ST-Link Utility.

**Instructions**: http://wiki.dragino.com/xwiki/bin/view/Main/Firmware%20Upgrade%20Instruction%20for%20STM32%20base%20products/

## Testing & Validation

### Run Comprehensive Tests
```
AT+STOP         # Stop radio
AT+TEST         # Run full validation suite
```

Expected output:
- ✅ State machine validation
- ✅ Error recovery validation
- ✅ Non-blocking delay validation
- ✅ Radio error simulation

### Monitor DI Queue
Enable verbose mode (build without `FAST_MODE`) and watch console during heartbeat cycles:
```
Radio busy during DI1 change - queuing state
DI1 change queued: state=1, time=1234
Processing queued DI changes (age: 856 ms)
Queued DI changes sent immediately
```

## Important Notes

### Radio Configuration
- **Asymmetric SF**: Transmitter uses SF7 (TX), SF8 (RX). Receiver uses SF8 (TX), SF7 (RX).
- **Frequency Pairing**: TX and RX frequencies must be swapped between transmitter/receiver.
- **TDC Guidelines**: 
  - `TDC=0`: Event-driven only (lowest latency, no heartbeat overhead)
  - `TDC=6000` to `60000`: Heartbeat mode (keeps link alive, enables DI queue)

### DI State Queue Behavior
- **Automatic**: Always active, no configuration needed
- **Transparent**: Normal DI changes work exactly as before
- **Queue Only When Needed**: Only captures state when radio is actively transmitting/receiving
- **Age Tracking**: Queue age is logged for diagnostics

### Compatibility
- ✅ Backward compatible with original AT commands
- ✅ Works with all group modes (P2P, P2MP)
- ✅ Compatible with original firmware (with reduced features)

## Documentation

- `DI_QUEUE_IMPLEMENTATION.md` - Complete DI state queue documentation
- `GITHUB_SETUP_GUIDE.md` - Git and GitHub setup instructions
- Original wiki: http://wiki.dragino.com/xwiki/bin/view/Main/

## Version History

### v1.4.0 (Nov 2025) - DI Queue Enhancement
- Added DI state change queue for heartbeat mode
- Eliminates heartbeat-cycle latency for DI changes
- Automatic queue processing after feedback
- Bi-directional queue support (TX and RX)

### v1.3.x (Oct 2025) - Reliability & Performance
- Enhanced error recovery system
- Non-blocking delays
- Bidirectional state mirroring
- AT+PRESET command
- Comprehensive testing suite
- FAST_MODE optimization

## Support & Issues

For bugs, feature requests, or questions, please open an issue on GitHub.

## License

Same as original DRAGINO LT-22222-L source code. 
