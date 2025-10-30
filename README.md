# LT-22222-L Enhanced LoRa P2P System

Enhanced firmware for DRAGINO LT-22222-L LoRa P2P communication with improved reliability and performance.

## 🎯 Key Enhancements

### Reliability Improvements
- ✅ **Race condition fixes** - Atomic state machine prevents data corruption
- ✅ **Infinite loop prevention** - Proper flag management
- ✅ **TX queueing system** - No lost packets during collisions
- ✅ **Error recovery** - Automatic retry with fallback parameters
- ✅ **Non-blocking delays** - System remains responsive

### Performance Optimizations
- ⚡ **Activity-based heartbeat** - TDC timer resets on DI activity
- ⚡ **FAST_MODE option** - Reduces latency by ~25-30ms
- ⚡ **Immediate feedback** - No artificial delays
- ⚡ **Smart TX scheduling** - Prevents heartbeat/interrupt collisions

### Features
- 🔧 **AT+TEST** - Comprehensive testing framework
- 🔧 **AT+STOP** - Emergency radio stop
- 🔧 **AT+PRESET** - One-command configuration
- 🔧 **Bidirectional feedback** - State mirroring between devices
- 🔧 **Direct DI→DO/RO mapping** - Flexible input/output control

## 📋 Quick Start

### Transmitter Configuration
```bash
AT+PRESET              # Base configuration
AT+TDC=5000           # Heartbeat every 5s (resets on DI activity)
AT+TRIG1=2,250        # DI1 triggers on both edges
AT+TRIG2=2,250        # DI2 triggers on both edges
ATZ                   # Reset to apply
```

### Receiver Configuration
```bash
AT+PRESET              # Base configuration
AT+TDC=0              # RX only mode
AT+DI1TODO1=1,0       # DI1 → DO1 (direct mapping)
AT+DI1TORO1=1,0       # DI1 → RO1 (direct mapping)
AT+DI2TODO2=1,0       # DI2 → DO2 (direct mapping)
AT+DI2TORO2=1,0       # DI2 → RO2 (direct mapping)
ATZ                   # Reset to apply
```

## ⚡ Performance Tuning

### Enable FAST_MODE (Optional)
Uncomment line 66 in `Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/src/main.c`:
```c
#define FAST_MODE  // Reduces latency by ~25-30ms
```

### Spreading Factor Trade-offs
```bash
# Maximum range (~2-5km):
AT+TXSF=12
AT+RXSF=12
Latency: ~800ms

# Balanced (~1-2km):
AT+TXSF=10
AT+RXSF=10
Latency: ~400ms

# Fast / Close range (<500m):
AT+TXSF=7
AT+RXSF=7
Latency: ~100ms
```

## 🔧 Compilation

### Option 1: Keil MDK-ARM (Recommended)
1. Open `Projects/Multi/Applications/LoRa/DRAGINO-LRWAN(AT)/MDK-ARM/sx1276mb1las/Project.uvprojx`
2. Press F7 to build
3. Press F8 to flash

**Note:** Free Keil MDK-Lite has 32KB code limit. Use optimization level O3 if needed.

### Option 2: STM32CubeIDE
1. Import existing project
2. Build with Release configuration
3. Flash via ST-Link

### Option 3: GCC (Free, no size limit)
See build scripts in project directory for GCC compilation.

## 📚 Documentation

### Original Documentation
- [Feature overview](http://wiki.dragino.com/xwiki/bin/view/Main/%20Point%20to%20Point%20Communication%20of%20LT-22222-L/)
- [Compile instructions](http://wiki.dragino.com/xwiki/bin/view/Main/Firmware%20Compile%20Instruction%20--%20STM32/)
- [Upgrade instructions](http://wiki.dragino.com/xwiki/bin/view/Main/Firmware%20Upgrade%20Instruction%20for%20STM32%20base%20products/)

## 🧪 Testing

Run comprehensive test suite:
```bash
AT+TEST    # Run full system test (radio must be idle)
AT+STOP    # Stop radio operations before testing
```

## 📊 Version History

### v1.3.4-Enhanced (Current)
- Race condition fixes with state machine
- TX queueing for collision prevention
- Activity-based heartbeat timer reset
- Direct DI→DO/RO mapping implementation
- FAST_MODE performance optimization
- Comprehensive error recovery system

### v1.3.4 (Original)
- Base DRAGINO firmware

## 🛠️ Advanced Configuration

### Mapping Modes
```
1 = Direct   (DI=HIGH → Output=HIGH)
2 = Inverse  (DI=HIGH → Output=LOW)
3 = Toggle   (DI=HIGH → Toggle output)
```

### Example Configurations
```bash
# Example 1: Direct mapping with delays
AT+DI1TODO1=1,5    # DI1 → DO1 direct, returns after 5s

# Example 2: Inverse mapping
AT+DI2TODO2=2,0    # DI2 → DO2 inverse, no auto-return

# Example 3: Toggle mode
AT+DI1TORO1=3,0    # DI1 → RO1 toggle on HIGH
```

## 📞 Support

For issues or questions related to:
- **Original hardware**: See DRAGINO documentation
- **Enhanced firmware**: Check commit history and comments in code

## ⚠️ Important Notes

1. **Always use matching GROUP_ID** on both devices (default: "HELLO123")
2. **TX and RX frequencies must be swapped** between transmitter and receiver
3. **Spreading factors must match** for communication
4. **Configure DI→DO/RO mappings** on receiver for outputs to work

## 📜 License

Based on STMicroelectronics code. See file headers for full license information. 
