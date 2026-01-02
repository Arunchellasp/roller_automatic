# HAL to LL Migration Guide

## Overview
This document explains how to use the new HAL/LL abstraction layer in your STM32F412 project.

## Files Created

### 1. **os_config.h**
Main configuration file to select between HAL and LL drivers.

**Location:** `Core/Inc/src/os/stm32/os_config.h`

```c
#define USE_HAL 1  // Set to 1 for HAL, 0 for LL
```

### 2. **os_stm32_ll_impl.h**
Low-level driver implementations using STM32 LL libraries.

**Location:** `Core/Inc/src/os/stm32/os_stm32_ll_impl.h`

Contains implementations for:
- GPIO operations (write, read, toggle)
- I2C master transmit/receive
- Timer operations
- UART send/receive with polling
- Delay functions

### 3. **os_stm32_core.h** (Updated)
Abstraction layer with conditional compilation.

**Location:** `Core/Inc/src/os/stm32/os_stm32_core.h`

## How to Switch Between HAL and LL

### Option 1: Use HAL (Default)
```c
// In os_config.h
#define USE_HAL 1
```

### Option 2: Use LL (Low-Level)
```c
// In os_config.h
#define USE_HAL 0
```

## Supported Features

### GPIO Operations
- `os_gpio_write()` - Write pin state
- `os_gpio_read()` - Read pin state
- `os_gpio_toggle()` - Toggle pin state

### I2C Operations
- `os_i2c_write8()` - Master transmit
- `os_i2c_read8()` - Master receive
- `os_i2c_write_register8()` - Write to device register
- `os_i2c_read_register8()` - Read from device register

### Timer Operations
- `os_timer_get_counter_32()` - Get counter value
- `os_timer_set_counter_32()` - Set counter value
- `os_timer_start()` / `os_timer_stop()` - Control timer

### UART Operations
- `os_uart_send()` - Send data (polling mode)
- `os_uart_receive()` - Receive data (polling mode)
- `os_uart_send_dma()` - Send via DMA (HAL only, falls back to polling for LL)
- `os_uart_receive_to_idle_dma()` - Receive until idle via DMA (HAL only)

### Delay
- `os_delay_ms(ms)` - Millisecond delay

## Implementation Details

### HAL Mode (USE_HAL = 1)
Uses standard STM32 HAL functions directly through the abstraction layer.
- Higher-level abstraction
- Easier to use
- Larger code footprint
- Better for quick development

### LL Mode (USE_HAL = 0)
Uses STM32 LL (Low-Level) driver functions.
- Direct register access
- Smaller code footprint
- More control over hardware
- Requires understanding of register operations

## Key Differences

### GPIO
**HAL:**
```c
HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
```

**LL:**
```c
LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
```

### I2C
**HAL:**
Uses handle-based approach with blocking calls
```c
HAL_I2C_Master_Transmit(hi2c, address, data, size, timeout);
```

**LL:**
Uses polling on status flags
```c
while (!LL_I2C_IsActiveFlag_TXE(i2c_instance)) { /* wait */ }
LL_I2C_TransmitData8(i2c_instance, byte);
```

### Timer
**HAL:**
```c
__HAL_TIM_GET_COUNTER(htim)
```

**LL:**
```c
LL_TIM_GetCounter(TIMx)
```

## Notes for LL Implementation

1. **I2C LL is Complex:**
   - Requires manual flag checking and state management
   - Basic blocking implementation provided
   - DMA mode not fully implemented
   - Consider using HAL for I2C if you need advanced features

2. **DMA for UART:**
   - LL DMA mode not fully configured in this implementation
   - UART DMA falls back to polling mode when using LL
   - For full DMA support with LL, additional DMA configuration needed

3. **Interrupt Handling:**
   - Timer interrupts still use HAL IRQHandler calls
   - LL mode requires manual ISR implementation if needed

4. **Timing:**
   - LL I2C and UART use `HAL_GetTick()` for timeouts
   - Ensure SysTick is properly configured

## Example Usage

### GPIO Example
```c
OS_Gpio led_pin;
os_gpio_init(&led_pin, (u32 *)GPIOA, GPIO_PIN_5);

// Works with both HAL and LL
os_gpio_write(&led_pin, 1);  // Set pin high
os_gpio_toggle(&led_pin);    // Toggle pin
b32 state = os_gpio_read(&led_pin);  // Read pin state
```

### I2C Example
```c
OS_I2c i2c_device;
os_i2c_init(&i2c_device, &hi2c1, 1000);  // Works with both HAL and LL

u8 data[4] = {0x01, 0x02, 0x03, 0x04};
if (os_i2c_write8(&i2c_device, 0x50, data, 4)) {
    // I2C write successful
}
```

### UART Example
```c
OS_Uart uart_port;
os_uart_init(&uart_port, &huart1, 1000);

u8 message[] = "Hello";
os_uart_send(&uart_port, message, 5);
```

## Compilation

No additional compilation steps needed. The conditional compilation is handled automatically based on the `USE_HAL` define.

To switch drivers:
1. Edit `os_config.h`
2. Change `#define USE_HAL` to 1 or 0
3. Rebuild project

## Troubleshooting

### LL I2C Not Working
- Verify I2C clock configuration is correct
- Check GPIO pins are configured as open-drain with pull-ups
- Ensure I2C timing parameters match your clock speed

### LL UART Issues
- Verify USART clock configuration
- Check baud rate calculations
- Ensure GPIO pins are correctly mapped

### Performance
- LL mode should be smaller and slightly faster
- Measure actual impact in your application
- Some feature combinations may require HAL mode

## Future Enhancements

Potential improvements:
1. Add LL DMA support for UART
2. Add SPI LL support
3. Add ADC LL support
4. Add I2C interrupt mode for LL
5. Optimization of LL I2C implementation
