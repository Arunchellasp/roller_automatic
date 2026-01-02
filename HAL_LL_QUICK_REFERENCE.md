# HAL/LL Abstraction Layer - Quick Reference

## Quick Start

### Change Driver Mode
Edit `Core/Inc/src/os/stm32/os_config.h`:
```c
#define USE_HAL 1   // Use HAL drivers
#define USE_HAL 0   // Use LL drivers
```

## API Reference

### GPIO Functions
```c
void os_gpio_init(OS_Gpio *gpio, u32 *port, i32 pin);
void os_gpio_write(OS_Gpio *gpio, b32 val);        // 0 = LOW, 1 = HIGH
b32 os_gpio_read(OS_Gpio *gpio);                   // Returns 0 or 1
void os_gpio_toggle(OS_Gpio *gpio);
```

### I2C Functions
```c
void os_i2c_init(OS_I2c *i2c, void *handle, u16 timeout_ms);

// Master write
b32 os_i2c_write8(OS_I2c *i2c, u16 device_address, u8* data, u16 count);

// Master read
b32 os_i2c_read8(OS_I2c *i2c, u16 device_address, u8* data, u16 count);

// Register-based operations
b32 os_i2c_write_register8(OS_I2c *i2c, u8 device_addr, u16 reg_addr, u8 *val, u16 count);
b32 os_i2c_read_register8(OS_I2c *i2c, u8 device_addr, u16 reg_addr, u8 *val, u16 count);
```

### Timer Functions
```c
void os_timer_init(OS_Timer *timer, void *handle, i32 channel, 
                   OS_Timer_Control *start, OS_Timer_Control *stop);
void os_timer_start(OS_Timer *timer);
void os_timer_stop(OS_Timer *timer);
i32 os_timer_get_counter_32(OS_Timer *timer);
void os_timer_set_counter_32(OS_Timer *timer, i32 val);
```

### UART Functions
```c
void os_uart_init(OS_Uart *uart, void *handle, u16 timeout);
b32 os_uart_send(OS_Uart *uart, u8 *data, u16 size);        // Returns 1 on success
void os_uart_receive(OS_Uart *uart, u8 *data, u16 size);
b32 os_uart_send_dma(OS_Uart *uart, u8 *data, u16 size);    // HAL only
void os_uart_receive_to_idle_dma(OS_Uart *uart, u8 *data, u16 size);  // HAL only
```

### Utility
```c
os_delay_ms(ms);  // Millisecond delay
```

## Code Examples

### Example 1: GPIO Blink
```c
#include "os_stm32_core.h"

OS_Gpio led;

int main(void) {
    os_gpio_init(&led, (u32 *)GPIOA, GPIO_PIN_5);
    
    while (1) {
        os_gpio_write(&led, 1);
        os_delay_ms(500);
        os_gpio_write(&led, 0);
        os_delay_ms(500);
    }
}
```

### Example 2: I2C Read/Write
```c
#include "os_stm32_core.h"

OS_I2c i2c;

int main(void) {
    os_i2c_init(&i2c, &hi2c1, 100);  // 100ms timeout
    
    u8 write_data[2] = {0x10, 0x20};
    u8 read_data[2];
    
    // Write to I2C device at address 0x50
    if (os_i2c_write8(&i2c, 0x50, write_data, 2)) {
        // Write successful
    }
    
    // Read from I2C device at address 0x50
    if (os_i2c_read8(&i2c, 0x50, read_data, 2)) {
        // Read successful
    }
}
```

### Example 3: UART Communication
```c
#include "os_stm32_core.h"

OS_Uart uart;

int main(void) {
    os_uart_init(&uart, &huart1, 1000);  // 1000ms timeout
    
    u8 message[] = "Hello STM32!";
    os_uart_send(&uart, message, 12);
    
    u8 buffer[32];
    os_uart_receive(&uart, buffer, 32);
}
```

### Example 4: Timer Operations
```c
#include "os_stm32_core.h"

OS_Timer timer;

int main(void) {
    os_timer_init(&timer, &htim2, 0, HAL_TIM_Base_Start, HAL_TIM_Base_Stop);
    
    os_timer_start(&timer);
    os_delay_ms(100);
    
    i32 count = os_timer_get_counter_32(&timer);
    os_timer_stop(&timer);
}
```

## Switching Between HAL and LL

### Method 1: Edit Configuration File
```bash
# Before compilation, edit:
Core/Inc/src/os/stm32/os_config.h

#define USE_HAL 1   # For HAL
#define USE_HAL 0   # For LL
```

### Method 2: Compiler Define (Optional)
Can be added to project makefile/CMakeLists.txt:
```makefile
CFLAGS += -DUSE_HAL=0
```

## Features by Mode

| Feature | HAL | LL |
|---------|-----|-----|
| GPIO | ✓ | ✓ |
| I2C Master | ✓ | ✓ |
| UART Polling | ✓ | ✓ |
| UART DMA | ✓ | ⚠ (fallback) |
| Timer | ✓ | ✓ |
| Code Size | Large | Small |
| Ease of Use | Easy | Complex |
| Performance | Good | Better |

## Performance Impact

- **LL mode**: ~5-10% smaller code size, slightly faster execution
- **HAL mode**: Larger binary, easier to debug and develop

## Troubleshooting

### "Undefined reference to os_ll_i2c_..."
- Ensure `#include "os_stm32_ll_impl.h"` is in os_stm32_core.h
- Check that USE_HAL is set to 0

### I2C timeout with LL
- Verify GPIO pins have pull-up resistors
- Check clock configuration
- Enable I2C clock in RCC

### UART not working
- Verify USART clock is enabled
- Check GPIO alternate function mapping
- Confirm baud rate configuration

## Files Location

| File | Path |
|------|------|
| Config | `Core/Inc/src/os/stm32/os_config.h` |
| Main API | `Core/Inc/src/os/stm32/os_stm32_core.h` |
| LL Impl | `Core/Inc/src/os/stm32/os_stm32_ll_impl.h` |
| Guide | `HAL_LL_MIGRATION_GUIDE.md` |
| Reference | `HAL_LL_QUICK_REFERENCE.md` |

## Support

For detailed information, see `HAL_LL_MIGRATION_GUIDE.md`
