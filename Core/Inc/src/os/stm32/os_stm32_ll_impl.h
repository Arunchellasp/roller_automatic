#ifndef OS_STM32_LL_IMPL_H
#define OS_STM32_LL_IMPL_H

#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_utils.h"

/**
 * @brief LL Implementation - GPIO operations
 */

#define OS_GPIO_WRITE(port, pin, val) \
	do { \
		if (val) { \
			LL_GPIO_SetOutputPin((GPIO_TypeDef *)port, pin); \
		} else { \
			LL_GPIO_ResetOutputPin((GPIO_TypeDef *)port, pin); \
		} \
	} while(0)

#define OS_GPIO_READ(port, pin) \
	LL_GPIO_IsInputPinSet((GPIO_TypeDef *)port, pin)

#define OS_GPIO_TOGGLE(port, pin) \
	LL_GPIO_TogglePin((GPIO_TypeDef *)port, pin)

/**
 * @brief LL Implementation - I2C operations
 * Note: LL I2C is more complex and requires manual state management
 */

typedef struct {
	I2C_TypeDef *instance;
	u16 timeout_ms;
	u32 start_time;
} OS_I2c_LL;

/**
 * @brief Check if I2C transfer is complete (timeout helper)
 */
internal inline b32
os_ll_i2c_is_timeout(OS_I2c_LL *i2c)
{
	// Simple timeout check - you may want to use SysTick for precision
	return (HAL_GetTick() - i2c->start_time) > i2c->timeout_ms;
}

/**
 * @brief LL I2C Master Transmit
 */
internal inline b32
os_ll_i2c_master_transmit(OS_I2c_LL *i2c, u16 device_address, u8 *data, u16 count)
{
	I2C_TypeDef *i2c_instance = i2c->instance;
	u16 index = 0;
	
	// Check if bus is busy
	if (LL_I2C_IsActiveFlag_BUSY(i2c_instance)) {
		return 0;
	}
	
	// Generate START condition
	LL_I2C_GenerateStartCondition(i2c_instance);
	i2c->start_time = HAL_GetTick();
	
	// Wait for START condition to be generated
	while (!LL_I2C_IsActiveFlag_SB(i2c_instance)) {
		if (os_ll_i2c_is_timeout(i2c)) return 0;
	}
	
	// Send slave address with write bit
	LL_I2C_TransmitData8(i2c_instance, (device_address << 1) & 0xFE);
	
	// Wait for address acknowledge
	i2c->start_time = HAL_GetTick();
	while (!LL_I2C_IsActiveFlag_ADDR(i2c_instance)) {
		if (os_ll_i2c_is_timeout(i2c)) return 0;
	}
	
	// Clear ADDR flag
	LL_I2C_ClearFlag_ADDR(i2c_instance);
	
	// Send data bytes
	for (index = 0; index < count; index++) {
		// Wait for TX buffer empty
		i2c->start_time = HAL_GetTick();
		while (!LL_I2C_IsActiveFlag_TXE(i2c_instance)) {
			if (os_ll_i2c_is_timeout(i2c)) return 0;
		}
		
		LL_I2C_TransmitData8(i2c_instance, data[index]);
	}
	
	// Wait for BTF (Byte Transfer Finished)
	i2c->start_time = HAL_GetTick();
	while (!LL_I2C_IsActiveFlag_BTF(i2c_instance)) {
		if (os_ll_i2c_is_timeout(i2c)) return 0;
	}
	
	// Generate STOP condition
	LL_I2C_GenerateStopCondition(i2c_instance);
	
	return 1;
}

/**
 * @brief LL I2C Master Receive
 */
internal inline b32
os_ll_i2c_master_receive(OS_I2c_LL *i2c, u16 device_address, u8 *data, u16 count)
{
	I2C_TypeDef *i2c_instance = i2c->instance;
	u16 index = 0;
	
	// Check if bus is busy
	if (LL_I2C_IsActiveFlag_BUSY(i2c_instance)) {
		return 0;
	}
	
	// Configure for reception (enable ACK)
	LL_I2C_AcknowledgeNextData(i2c_instance, LL_I2C_ACK);
	
	// Generate START condition
	LL_I2C_GenerateStartCondition(i2c_instance);
	i2c->start_time = HAL_GetTick();
	
	// Wait for START condition
	while (!LL_I2C_IsActiveFlag_SB(i2c_instance)) {
		if (os_ll_i2c_is_timeout(i2c)) return 0;
	}
	
	// Send slave address with read bit
	LL_I2C_TransmitData8(i2c_instance, (device_address << 1) | 0x01);
	
	// Wait for address acknowledge
	i2c->start_time = HAL_GetTick();
	while (!LL_I2C_IsActiveFlag_ADDR(i2c_instance)) {
		if (os_ll_i2c_is_timeout(i2c)) return 0;
	}
	
	// Clear ADDR flag
	LL_I2C_ClearFlag_ADDR(i2c_instance);
	
	// Receive data bytes
	for (index = 0; index < count; index++) {
		if (index == count - 1) {
			// Last byte: disable ACK before receiving
			LL_I2C_AcknowledgeNextData(i2c_instance, LL_I2C_NACK);
		}
		
		// Wait for RX buffer full
		i2c->start_time = HAL_GetTick();
		while (!LL_I2C_IsActiveFlag_RXNE(i2c_instance)) {
			if (os_ll_i2c_is_timeout(i2c)) return 0;
		}
		
		data[index] = LL_I2C_ReceiveData8(i2c_instance);
	}
	
	// Generate STOP condition
	LL_I2C_GenerateStopCondition(i2c_instance);
	
	return 1;
}

/**
 * @brief LL Timer Operations
 */

#define OS_TIM_GET_COUNTER(htim) \
	LL_TIM_GetCounter((TIM_TypeDef *)htim)

#define OS_TIM_SET_COUNTER(htim, val) \
	LL_TIM_SetCounter((TIM_TypeDef *)htim, val)

#define OS_TIM_START(htim) \
	LL_TIM_EnableCounter((TIM_TypeDef *)htim)

#define OS_TIM_STOP(htim) \
	LL_TIM_DisableCounter((TIM_TypeDef *)htim)

#define OS_TIM_START_IT(htim) \
	do { \
		LL_TIM_EnableIT_UPDATE((TIM_TypeDef *)htim); \
		LL_TIM_EnableCounter((TIM_TypeDef *)htim); \
	} while(0)

#define OS_TIM_STOP_IT(htim) \
	do { \
		LL_TIM_DisableIT_UPDATE((TIM_TypeDef *)htim); \
		LL_TIM_DisableCounter((TIM_TypeDef *)htim); \
	} while(0)

/**
 * @brief LL UART Operations
 */

#define OS_UART_SEND(huart, data, size) \
	os_ll_uart_transmit((USART_TypeDef *)huart, data, size, 1000)

#define OS_UART_RECEIVE(huart, data, size) \
	os_ll_uart_receive((USART_TypeDef *)huart, data, size, 1000)

/**
 * @brief LL UART Transmit implementation
 */
internal inline b32
os_ll_uart_transmit(USART_TypeDef *usart_instance, u8 *data, u16 size, u16 timeout_ms)
{
	u16 index = 0;
	u32 start_time = HAL_GetTick();
	
	for (index = 0; index < size; index++) {
		// Wait for TX buffer empty with timeout
		while (!LL_USART_IsActiveFlag_TXE(usart_instance)) {
			if ((HAL_GetTick() - start_time) > timeout_ms) {
				return 0;
			}
		}
		LL_USART_TransmitData8(usart_instance, data[index]);
	}
	
	// Wait for transmission complete (TC flag)
	start_time = HAL_GetTick();
	while (!LL_USART_IsActiveFlag_TC(usart_instance)) {
		if ((HAL_GetTick() - start_time) > timeout_ms) {
			return 0;
		}
	}
	
	return 1;
}

/**
 * @brief LL UART Receive implementation
 */
internal inline b32
os_ll_uart_receive(USART_TypeDef *usart_instance, u8 *data, u16 size, u16 timeout_ms)
{
	u16 index = 0;
	u32 start_time = HAL_GetTick();
	
	for (index = 0; index < size; index++) {
		// Wait for RX buffer full with timeout
		start_time = HAL_GetTick();
		while (!LL_USART_IsActiveFlag_RXNE(usart_instance)) {
			if ((HAL_GetTick() - start_time) > timeout_ms) {
				return 0;
			}
		}
		data[index] = LL_USART_ReceiveData8(usart_instance);
	}
	
	return 1;
}

/**
 * @brief LL Delay implementation
 */
#define OS_DELAY_MS(ms) LL_mDelay(ms)

#endif // OS_STM32_LL_IMPL_H
