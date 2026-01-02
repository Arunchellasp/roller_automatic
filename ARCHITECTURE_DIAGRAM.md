# HAL/LL Architecture Diagram

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      APPLICATION CODE                           │
│                  (Uses os_stm32_core.h API)                     │
│                                                                   │
│  Example: os_gpio_write(), os_uart_send(), os_i2c_read8()       │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│              os_stm32_core.h (ABSTRACTION LAYER)                │
│                                                                   │
│   Contains #ifdef USE_HAL conditional compilation              │
│   - All API functions                                           │
│   - Type definitions that adapt to HAL or LL                   │
└────────┬────────────────────────────────────┬────────────────────┘
         │                                    │
    USE_HAL=1                            USE_HAL=0
         │                                    │
         ▼                                    ▼
┌──────────────────────────┐    ┌──────────────────────────────┐
│   HAL Mode Implementation │    │   LL Mode Implementation     │
│                          │    │                              │
│ os_stm32_core.h calls:   │    │ os_stm32_ll_impl.h contains: │
│                          │    │                              │
│ • HAL_GPIO_WritePin()    │    │ • LL_GPIO_SetOutputPin()     │
│ • HAL_I2C_Master_Tx()    │    │ • LL_I2C_TransmitData8()     │
│ • HAL_UART_Transmit()    │    │ • LL_USART_TransmitData8()   │
│ • HAL_TIM functions      │    │ • LL_TIM_GetCounter()        │
│ • HAL_Delay()            │    │ • LL_mDelay()                │
└──────────────┬───────────┘    └──────────────┬───────────────┘
               │                               │
               ▼                               ▼
    ┌──────────────────────┐        ┌──────────────────────┐
    │  STM32 HAL Libraries │        │  STM32 LL Libraries  │
    │                      │        │                      │
    │ stm32f4xx_hal_*.h    │        │ stm32f4xx_ll_*.h     │
    └──────────────┬───────┘        └──────────────┬───────┘
                   │                               │
                   └───────────────┬───────────────┘
                                   │
                                   ▼
                    ┌──────────────────────────────┐
                    │   STM32F412 Hardware Registers │
                    │   (GPIO, UART, I2C, Timer)    │
                    └──────────────────────────────┘
```

## Configuration Flow

```
Developer edits os_config.h
              │
              ▼
    #define USE_HAL 1 or 0
              │
              ▼
    Compiler preprocessor processes:
    - os_stm32_core.h #if USE_HAL
    - Includes appropriate implementation
              │
    ┌─────────┴────────────┐
    │                      │
    ▼                      ▼
  HAL Path              LL Path
  Direct calls          Calls to os_stm32_ll_impl.h
    │                      │
    └─────────────┬────────┘
                  │
                  ▼
          Compiled Binary
```

## File Organization

```
Core/
├── Inc/
│   ├── stm32f4xx_hal_conf.h
│   └── src/
│       └── os/
│           └── stm32/
│               ├── os_config.h                    [MAIN CONFIG]
│               ├── os_stm32_core.h                [ABSTRACTION API]
│               └── os_stm32_ll_impl.h             [LL IMPLEMENTATIONS]
│
└── Src/
    ├── stm32f4xx_hal_msp.c
    ├── stm32f4xx_it.c
    ├── main.c                                     [YOUR APPLICATION]
    └── ...

[ROOT]
├── IMPLEMENTATION_SUMMARY.md                      [THIS DOCUMENT]
├── HAL_LL_MIGRATION_GUIDE.md                      [DETAILED GUIDE]
└── HAL_LL_QUICK_REFERENCE.md                      [QUICK API REFERENCE]
```

## Decision Tree for Mode Selection

```
                    Use HAL or LL?
                          │
                          ▼
              ┌─────────────────────────┐
              │  Performance Critical?   │
              └─────────────────────────┘
                    │            │
                  YES           NO
                    │            │
                    ▼            ▼
                  Use LL      Is Code    
                              Size
                            Critical?
                              │    │
                            YES   NO
                              │    │
                              ▼    ▼
                            Use LL Use HAL
                                (easier)
```

## Function Flow Example: GPIO Write

### HAL Mode (USE_HAL = 1)
```
Application calls:
  os_gpio_write(&led, 1)
        │
        ▼
  #if USE_HAL branch taken
        │
        ▼
  HAL_GPIO_WritePin(gpio->port, gpio->pin, 
                   GPIO_PIN_SET)
        │
        ▼
  STM32 HAL Library handles register writes
        │
        ▼
  Hardware GPIO pin set HIGH
```

### LL Mode (USE_HAL = 0)
```
Application calls:
  os_gpio_write(&led, 1)
        │
        ▼
  #else branch taken
        │
        ▼
  OS_GPIO_WRITE(gpio->port, gpio->pin, 1)
        │
        ▼
  LL_GPIO_SetOutputPin(gpio->port, gpio->pin)
        │
        ▼
  Direct register manipulation via LL macros
        │
        ▼
  Hardware GPIO pin set HIGH
```

## Conditional Compilation Visual

```
Source File (os_stm32_core.h):
┌────────────────────────────────────┐
│ #include "os_config.h"             │
│ #if !USE_HAL                       │
│ #include "os_stm32_ll_impl.h"      │
│ #endif                             │
│                                    │
│ void os_gpio_write(...) {          │
│   #if USE_HAL                      │
│   HAL_GPIO_WritePin(...);          │ ◄─── INCLUDED
│   #else                            │
│   OS_GPIO_WRITE(...);              │ ◄─── EXCLUDED
│   #endif                           │
│ }                                  │
└────────────────────────────────────┘
        │
        ▼
  C Preprocessor
        │
        ▼
Compiled Binary (only HAL path included):
┌────────────────────────────────────┐
│ void os_gpio_write(...) {          │
│   HAL_GPIO_WritePin(...);          │
│ }                                  │
└────────────────────────────────────┘
```

## Supported Peripherals

```
GPIO                  I2C                    Timer               UART
├─ Write Pin          ├─ Master Tx            ├─ Start/Stop       ├─ Send (Polling)
├─ Read Pin           ├─ Master Rx            ├─ Get Counter      ├─ Receive (Polling)
└─ Toggle Pin         ├─ Write Register       ├─ Set Counter      ├─ Send DMA*
                      └─ Read Register        └─ IT Control       └─ Receive DMA*

* DMA features available in HAL mode only
```

## Performance Impact

```
Code Size Comparison:
┌─────────────────────────┐
│ HAL Mode:               │ ████████████████ (100%)
├─────────────────────────┤
│ LL Mode:                │ ██████████ (~65%)
└─────────────────────────┘

Execution Speed:
┌─────────────────────────┐
│ HAL Mode:               │ ████████████ (standard)
├─────────────────────────┤
│ LL Mode:                │ █████████████ (5-15% faster)
└─────────────────────────┘

Ease of Use:
┌─────────────────────────┐
│ HAL Mode:               │ ████████████ (easier)
├─────────────────────────┤
│ LL Mode:                │ ████████ (more complex)
└─────────────────────────┘
```

## Type Adaptation

```
Type Definition Adaptation in os_stm32_core.h:

┌──────────────────────────────────────────┐
│ typedef struct OS_I2c {                  │
│ #if USE_HAL                              │
│   I2C_HandleTypeDef *handle;   ◄─ HAL    │
│ #else                                    │
│   I2C_TypeDef *handle;         ◄─ LL     │
│ #endif                                   │
│   u16 timeout_ms;                        │
│ } OS_I2c;                                │
└──────────────────────────────────────────┘

The same structure adapts to both modes!
```

## Initialization Pattern

```
Both modes use same initialization:

Initialize:
  os_gpio_init(&gpio, port, pin)
  os_uart_init(&uart, handle, timeout)
  os_i2c_init(&i2c, handle, timeout)

Behind the scenes:
  ┌─────────────────────┐
  │ Receives handle     │
  │ (cast to void *)    │
  ├─────────────────────┤
  │ HAL Mode:           │
  │ Cast to HAL type    │
  │ Use HAL function    │
  ├─────────────────────┤
  │ LL Mode:            │
  │ Cast to LL type     │
  │ Use LL register ops │
  └─────────────────────┘
```

---

**Key Takeaway:** The abstraction layer acts as a bridge between your application and either the HAL or LL drivers, controlled by a single `#define`.
