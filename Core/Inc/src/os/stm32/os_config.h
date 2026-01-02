#ifndef OS_CONFIG_H
#define OS_CONFIG_H

/**
 * @brief Select between HAL (Hardware Abstraction Layer) and LL (Low Level) drivers
 * 
 * Set to 1 to use HAL drivers (higher-level abstraction, easier to use)
 * Set to 0 to use LL drivers (low-level, more control and lighter footprint)
 */
#define USE_HAL 1

#endif // OS_CONFIG_H
