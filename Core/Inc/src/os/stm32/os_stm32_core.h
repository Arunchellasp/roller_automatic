
#ifndef OS_STM32_CORE_H
#define OS_STM32_CORE_H

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
	HAL_GPIO_WritePin(gpio->port, gpio->pin, (val == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

internal inline b32
os_gpio_read(OS_Gpio *gpio)
{
	b32 result = HAL_GPIO_ReadPin(gpio->port, gpio->pin);
	return result;
}

internal inline void
os_gpio_toggle(OS_Gpio *gpio)
{
	HAL_GPIO_TogglePin(gpio->port, gpio->pin);
}

// I2C

typedef struct OS_I2c OS_I2c;
struct OS_I2c
{
	I2C_HandleTypeDef *handle;
	u16 timeout_ms;
};

internal inline void 
os_i2c_init(OS_I2c *i2c, I2C_HandleTypeDef *handle, u16 timeout_ms)
{
	Assert(i2c != 0);
    i2c->handle = handle;
	i2c->timeout_ms = timeout_ms;
}

internal inline b32
os_i2c_write8(OS_I2c *i2c, u16 device_address, u8* data, u16 count)
{
	HAL_StatusTypeDef result = HAL_I2C_Master_Transmit(i2c->handle, device_address, data, count, i2c->timeout_ms);
	return result == HAL_OK;
};

internal inline b32
os_i2c_read8(OS_I2c *i2c, u16 device_address, u8* data, u16 count)
{
	HAL_StatusTypeDef result = HAL_I2C_Master_Receive(i2c->handle, device_address, data, count, i2c->timeout_ms);
	return result == HAL_OK;
};

internal inline b32
os_i2c_write_register8(OS_I2c *i2c, u8 device_address, u16 register_address, u8 *val, u16 count)
{
	HAL_StatusTypeDef result = HAL_I2C_Mem_Write(i2c->handle, device_address, register_address, I2C_MEMADD_SIZE_8BIT, val, count, i2c->timeout_ms);
	return result == HAL_OK;
}

internal inline b32
os_i2c_read_register8(OS_I2c *i2c, u8 device_address, u16 register_address, u8 *val, u16 count)
{
	HAL_StatusTypeDef result = HAL_I2C_Mem_Read(i2c->handle, device_address, register_address, I2C_MEMADD_SIZE_8BIT, val, count, i2c->timeout_ms);
	return result == HAL_OK;
}

//
//~ rsb: Timer
//

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
	i32 result = __HAL_TIM_GET_COUNTER(timer->handle);
	return result;
}

internal inline void
os_timer_set_counter_32(OS_Timer *timer, i32 val)
{
	__HAL_TIM_SET_COUNTER(timer->handle, val);
}

internal inline TIM_HandleTypeDef *
os_timer_handle(OS_Timer *timer)
{
    return timer->handle;
}

//
//~ rsb: Timer IT
// 

typedef HAL_StatusTypeDef OS_Timer_It_Control(TIM_HandleTypeDef *);
typedef struct OS_Timer_It OS_Timer_It;
struct OS_Timer_It
{
	TIM_HandleTypeDef *handle;
    OS_Timer_It_Control *start;
    OS_Timer_It_Control *stop;
};

internal inline void 
os_timer_it_init(OS_Timer_It *timer, TIM_HandleTypeDef *handle, OS_Timer_It_Control *start, OS_Timer_It_Control *stop)
{
	Assert(timer != 0);
    timer->handle = handle;
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
    return timer->handle;
}

//
//~ rsb: Uart
//

typedef struct OS_Uart OS_Uart;
struct OS_Uart
{
	UART_HandleTypeDef *handle;
    u16 timeout;
};

internal inline void
os_uart_init(OS_Uart *uart, UART_HandleTypeDef *handle, u16 timeout)
{
	Assert(uart != 0);
    uart->handle = handle;
    uart->timeout = timeout;
}

internal inline b32
os_uart_send(OS_Uart *uart, u8 *data, u16 size)
{
	HAL_StatusTypeDef status = HAL_UART_Transmit(uart->handle, data, size, uart->timeout);
	return (status == HAL_OK) ? 1 : 0;
}

internal void
os_uart_receive(OS_Uart *uart, u8 *data, u16 size)
{
	HAL_UART_Receive(uart->handle, data, size, uart->timeout);
}

internal inline b32
os_uart_send_dma(OS_Uart *uart, u8 *data, u16 size)
{
	HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(uart->handle, data, size);
	return (status == HAL_OK) ? 1 : 0;
}

internal void
os_uart_receive_to_idle_dma(OS_Uart *uart, u8 *data, u16 size)
{
	HAL_UARTEx_ReceiveToIdle_DMA(uart->handle, data, size);
}

internal inline UART_HandleTypeDef *
os_uart_handle(OS_Uart *uart)
{
    return uart->handle;
}

//
//~ rsb: general
//
#define os_delay_ms(ms) HAL_Delay(ms)

#endif //STM32_H
