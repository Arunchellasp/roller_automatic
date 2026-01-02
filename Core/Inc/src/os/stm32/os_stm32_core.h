
#ifndef OS_STM32_CORE_H
#define OS_STM32_CORE_H

#include "os_config.h"

#if !USE_HAL
#include "os_stm32_ll_impl.h"
#endif

//
//~ rsb: GPIO
//

typedef struct OS_Gpio OS_Gpio;
struct OS_Gpio
{
	GPIO_TypeDef *port;
    i32 pin;
};

#define stm32_gpio(name) (u32 *)(name##_GPIO_Port), (i32)(name##_Pin)
internal inline void
os_gpio_init(OS_Gpio *gpio, u32 *port, i32 pin)
{
	Assert(gpio != 0);
	gpio->port = (GPIO_TypeDef *)port;
	gpio->pin = pin;
}

internal inline void
os_gpio_write(OS_Gpio *gpio, b32 val)
{
#if USE_HAL
	HAL_GPIO_WritePin(gpio->port, gpio->pin, (val == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
#else
	OS_GPIO_WRITE(gpio->port, gpio->pin, val);
#endif
}

internal inline b32
os_gpio_read(OS_Gpio *gpio)
{
#if USE_HAL
	b32 result = HAL_GPIO_ReadPin(gpio->port, gpio->pin);
	return result;
#else
	return OS_GPIO_READ(gpio->port, gpio->pin);
#endif
}

internal inline void
os_gpio_toggle(OS_Gpio *gpio)
{
#if USE_HAL
	HAL_GPIO_TogglePin(gpio->port, gpio->pin);
#else
	OS_GPIO_TOGGLE(gpio->port, gpio->pin);
#endif
}

// I2C

typedef struct OS_I2c OS_I2c;
struct OS_I2c
{
#if USE_HAL
	I2C_HandleTypeDef *handle;
#else
	I2C_TypeDef *handle;
#endif
	u16 timeout_ms;
};

internal inline void 
os_i2c_init(OS_I2c *i2c, void *handle, u16 timeout_ms)
{
	Assert(i2c != 0);
    i2c->handle = (void *)handle;
	i2c->timeout_ms = timeout_ms;
}

internal inline b32
os_i2c_write8(OS_I2c *i2c, u16 device_address, u8* data, u16 count)
{
#if USE_HAL
	HAL_StatusTypeDef result = HAL_I2C_Master_Transmit((I2C_HandleTypeDef *)i2c->handle, device_address, data, count, i2c->timeout_ms);
	return result == HAL_OK;
#else
	OS_I2c_LL i2c_ll = {.instance = (I2C_TypeDef *)i2c->handle, .timeout_ms = i2c->timeout_ms};
	return os_ll_i2c_master_transmit(&i2c_ll, device_address, data, count);
#endif
};

internal inline b32
os_i2c_read8(OS_I2c *i2c, u16 device_address, u8* data, u16 count)
{
#if USE_HAL
	HAL_StatusTypeDef result = HAL_I2C_Master_Receive((I2C_HandleTypeDef *)i2c->handle, device_address, data, count, i2c->timeout_ms);
	return result == HAL_OK;
#else
	OS_I2c_LL i2c_ll = {.instance = (I2C_TypeDef *)i2c->handle, .timeout_ms = i2c->timeout_ms};
	return os_ll_i2c_master_receive(&i2c_ll, device_address, data, count);
#endif
};

internal inline b32
os_i2c_write_register8(OS_I2c *i2c, u8 device_address, u16 register_address, u8 *val, u16 count)
{
#if USE_HAL
	HAL_StatusTypeDef result = HAL_I2C_Mem_Write((I2C_HandleTypeDef *)i2c->handle, device_address, register_address, I2C_MEMADD_SIZE_8BIT, val, count, i2c->timeout_ms);
	return result == HAL_OK;
#else
	// LL Implementation: Write to device register address first, then write data
	OS_I2c_LL i2c_ll = {.instance = (I2C_TypeDef *)i2c->handle, .timeout_ms = i2c->timeout_ms};
	u8 addr_byte = (u8)register_address;
	if (!os_ll_i2c_master_transmit(&i2c_ll, device_address, &addr_byte, 1)) {
		return 0;
	}
	return os_ll_i2c_master_transmit(&i2c_ll, device_address, val, count);
#endif
}

internal inline b32
os_i2c_read_register8(OS_I2c *i2c, u8 device_address, u16 register_address, u8 *val, u16 count)
{
#if USE_HAL
	HAL_StatusTypeDef result = HAL_I2C_Mem_Read((I2C_HandleTypeDef *)i2c->handle, device_address, register_address, I2C_MEMADD_SIZE_8BIT, val, count, i2c->timeout_ms);
	return result == HAL_OK;
#else
	// LL Implementation: Write register address first, then read data
	OS_I2c_LL i2c_ll = {.instance = (I2C_TypeDef *)i2c->handle, .timeout_ms = i2c->timeout_ms};
	u8 addr_byte = (u8)register_address;
	if (!os_ll_i2c_master_transmit(&i2c_ll, device_address, &addr_byte, 1)) {
		return 0;
	}
	return os_ll_i2c_master_receive(&i2c_ll, device_address, val, count);
#endif
}

//
//~ rsb: Timer
//

#if USE_HAL
typedef HAL_StatusTypeDef OS_Timer_Control(TIM_HandleTypeDef *, u32);
typedef struct OS_Timer OS_Timer;
struct OS_Timer
{
    TIM_HandleTypeDef *handle;
    i32 channel;
    OS_Timer_Control *start;
    OS_Timer_Control *stop;
};

#define stm32_timer(pre, post) pre##Start##post, pre##Stop##post
#define stm32_timer_it stm32_timer(HAL_TIM_Base_, _IT)
#define stm32_timer_encoder TIM_CHANNEL_ALL, stm32_timer(HAL_TIM_Encoder_, )
#else
typedef void OS_Timer_Control(TIM_TypeDef *, u32);
typedef struct OS_Timer OS_Timer;
struct OS_Timer
{
    TIM_TypeDef *handle;
    i32 channel;
    OS_Timer_Control *start;
    OS_Timer_Control *stop;
};

// LL timer control stubs (defined inline below)
#define stm32_timer(pre, post) 0, 0
#define stm32_timer_it stm32_timer(LL_TIM_, )
#define stm32_timer_encoder 0, stm32_timer(LL_TIM_, )
#endif
internal inline void
os_timer_init(OS_Timer *timer, TIM_HandleTypeDef *handle, i32 channel, OS_Timer_Control *start, OS_Timer_Control *stop)
{
	Assert(timer != 0);
    timer->handle = handle;
    timer->channel = channel;
    timer->start = start;
    timer->stop = stop;
} 

internal inline void
os_timer_start(OS_Timer *timer)
{
    timer->start(timer->handle, timer->channel);
}

internal inline void
os_timer_stop(OS_Timer *timer)
{
    timer->stop(timer->handle, timer->channel);
}

internal inline i32
os_timer_get_counter_32(OS_Timer *timer)
{
#if USE_HAL
	i32 result = __HAL_TIM_GET_COUNTER(timer->handle);
	return result;
#else
	return LL_TIM_GetCounter((TIM_TypeDef *)timer->handle);
#endif
}

internal inline void
os_timer_set_counter_32(OS_Timer *timer, i32 val)
{
#if USE_HAL
	__HAL_TIM_SET_COUNTER(timer->handle, val);
#else
	LL_TIM_SetCounter((TIM_TypeDef *)timer->handle, val);
#endif
}

internal inline TIM_HandleTypeDef *
os_timer_handle(OS_Timer *timer)
{
    return (TIM_HandleTypeDef *)timer->handle;
}

//
//~ rsb: Timer IT
// 

#if USE_HAL
typedef HAL_StatusTypeDef OS_Timer_It_Control(TIM_HandleTypeDef *);
#else
typedef void OS_Timer_It_Control(TIM_TypeDef *);
#endif

typedef struct OS_Timer_It OS_Timer_It;
struct OS_Timer_It
{
#if USE_HAL
	TIM_HandleTypeDef *handle;
#else
	TIM_TypeDef *handle;
#endif
    OS_Timer_It_Control *start;
    OS_Timer_It_Control *stop;
};

internal inline void 
os_timer_it_init(OS_Timer_It *timer, void *handle, OS_Timer_It_Control *start, OS_Timer_It_Control *stop)
{
	Assert(timer != 0);
    timer->handle = (void *)handle;
    timer->start = start;
    timer->stop = stop;
}

internal inline void 
os_timer_it_start(OS_Timer_It *timer)
{
    timer->start(timer->handle);
}

internal inline void 
os_timer_it_stop(OS_Timer_It *timer)
{
    timer->stop(timer->handle);
}

internal inline TIM_HandleTypeDef *
os_timer_it_handle(OS_Timer_It *timer)
{
    return (TIM_HandleTypeDef *)timer->handle;
}

//
//~ rsb: Uart
//

typedef struct OS_Uart OS_Uart;
struct OS_Uart
{
#if USE_HAL
	UART_HandleTypeDef *handle;
#else
	USART_TypeDef *handle;
#endif
    u16 timeout;
};

internal inline void
os_uart_init(OS_Uart *uart, void *handle, u16 timeout)
{
	Assert(uart != 0);
    uart->handle = (void *)handle;
    uart->timeout = timeout;
}

internal inline b32
os_uart_send(OS_Uart *uart, u8 *data, u16 size)
{
#if USE_HAL
	HAL_StatusTypeDef status = HAL_UART_Transmit((UART_HandleTypeDef *)uart->handle, data, size, uart->timeout);
	return (status == HAL_OK) ? 1 : 0;
#else
	return os_ll_uart_transmit((USART_TypeDef *)uart->handle, data, size, uart->timeout);
#endif
}

internal void
os_uart_receive(OS_Uart *uart, u8 *data, u16 size)
{
#if USE_HAL
	HAL_UART_Receive((UART_HandleTypeDef *)uart->handle, data, size, uart->timeout);
#else
	os_ll_uart_receive((USART_TypeDef *)uart->handle, data, size, uart->timeout);
#endif
}

internal inline b32
os_uart_send_dma(OS_Uart *uart, u8 *data, u16 size)
{
#if USE_HAL
	HAL_StatusTypeDef status = HAL_UART_Transmit_DMA((UART_HandleTypeDef *)uart->handle, data, size);
	return (status == HAL_OK) ? 1 : 0;
#else
	// LL DMA mode not implemented in basic LL - requires DMA setup
	// For now, fall back to polling mode
	return os_ll_uart_transmit((USART_TypeDef *)uart->handle, data, size, uart->timeout);
#endif
}

internal void
os_uart_receive_to_idle_dma(OS_Uart *uart, u8 *data, u16 size)
{
#if USE_HAL
	HAL_UARTEx_ReceiveToIdle_DMA((UART_HandleTypeDef *)uart->handle, data, size);
#else
	// LL DMA mode not implemented - requires DMA setup
	// For now, fall back to polling mode
	os_ll_uart_receive((USART_TypeDef *)uart->handle, data, size, uart->timeout);
#endif
}

internal inline UART_HandleTypeDef *
os_uart_handle(OS_Uart *uart)
{
    return (UART_HandleTypeDef *)uart->handle;
}

//
//~ rsb: general
//
#if USE_HAL
#define os_delay_ms(ms) HAL_Delay(ms)
#else
#define os_delay_ms(ms) LL_mDelay(ms)
#endif

#endif //STM32_H
