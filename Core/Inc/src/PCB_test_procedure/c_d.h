/* date = September 30th 2025 10:43 am */

#ifndef C_D_H
#define C_D_H






internal void
aw_init()
{
	os_delay_ms(3000);
	
	
	{
		for(u32 it = 0; it < ArrayCount(thread_arena); it += 1)
		{
			thread_arena[it] = arena_alloc(KB(1));
		}
	}
	
    
	
	Arena *arena = arena_alloc(KB(10));
	aw = push_array(arena, AW_State, 1);
	aw->arena = arena;
	
    
    
    
	// NOTE(ARUN): @load_ADC
	ADS1115_init(&aw->tension_sensor.differential_amplifier, ADS1115_ADDR_GND, 0, ADS1115_GAIN_SIXTEEN, 0.0078125, ADS1115_CONVERSIONDELAY);
	aw->os.differential_amplifier = push_array(arena, OS_I2c,1);
	os_i2c_init(aw->os.differential_amplifier, &hi2c1, 10);
	TAS501_init(&aw->tension_sensor.strain_gauge, 5.0f, 200.0f, 2.0f);
	
	
    // NOTE(ARUN): @app_communication
    aw->os.app_serial = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.app_serial, &huart1, APP_COMMS_RESPONSE_TIMEOUT);
    comms_arch_init(&aw->comms_arch, APP_COMMS_SUPPORTED_ARCHITECURE, APP_COMMS_SUPPORTED_ARCHITECURE_VERSION);
	os_uart_receive_to_idle_dma(aw->os.app_serial, (u8 *)(&aw->comms_rx), sizeof(aw->comms_rx)*2);
    
    
    
#if 0    
    // NOTE(ARUN): @Endobot_power_control
    aw->os.Endo70V_toggle = push_array(arena, OS_Gpio, 1);
	aw->os.Endo130V_toggle = push_array(arena, OS_Gpio, 1);
#endif
    
    
    
    // NOTE(ARUN):  @com_controll_init
	aw->os.drum_com_toggle= push_array(arena, OS_Gpio, 1);
	os_gpio_init(aw->os.drum_com_toggle,stm32_gpio(EN_DRUM_COM));
    
	aw->os.spooler_com_toggle = push_array(arena, OS_Gpio, 1);
	os_gpio_init(aw->os.spooler_com_toggle,stm32_gpio(EN_SPOOLER_COM));
    
	// NOTE(ARUN): @motor_com
	aw->os.modbus_drum = push_array(arena, OS_Uart, 1);
    os_uart_init(aw->os.modbus_drum, &huart6, MODBUS_RESPONSE_TIMEOUT);
	aw->os.modbus_spooler = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.modbus_spooler, &huart3, MODBUS_RESPONSE_TIMEOUT);
    
    
    
    os_gpio_write(aw->os.drum_com_toggle,1);
    os_gpio_write(aw->os.spooler_com_toggle,1);
    os_delay_ms(100);
    motor_activate(aw->os.modbus_drum, aw->os.modbus_spooler, &aw->motor_state, 1);
    
    
    
    
#if 0    
	os_gpio_write(aw->os.Endo70V_toggle,1);
    os_gpio_write(aw->os.Endo130V_toggle,1);
#endif
    
    
    //os_gpio_write(aw->os.drum_com_toggle,0);
	// os_gpio_write(aw->os.spooler_com_toggle,0);
    
    //os_delay_ms(10);
    
    
    
	//- rsb: encoder init
	aw->spooler_encoder_ppr = 4000.0f;
	aw->spooler_circumference_m = PI * 0.05f;
	aw->os.spooler_encoder = push_array(arena, OS_Timer, 1);
	os_timer_init(aw->os.spooler_encoder, &htim3, stm32_timer_encoder);
	os_timer_start(aw->os.spooler_encoder);
	
	//- rsb: control system init
	r32 timer_dt = 1.0f / 100.0f;
	aw->control_system_dt = timer_dt;
    
    
    
	//TIMER IT
	aw->os.control_system = push_array(arena, OS_Timer_It, 1);
	os_timer_it_init(aw->os.control_system ,&htim1, stm32_timer_it);
	os_timer_it_start(aw->os.control_system);
    
}


internal r32
get_encoder_turn_count(OS_Timer *timer, r32 ppr)
{
	i32 ticks_elapsed = os_timer_get_counter_32(timer);
	os_timer_set_counter_32(timer, 0);
	r32 turn_count = (r32)ticks_elapsed / ppr;
	return turn_count;
}



internal void
aw_frame()
{
    
    OS_State *os = &aw->os;
    
    
    r32 actual_tension = {0};
    r32 differential_amplifier_mV = ADS1115_read_adc_diff_01(&aw->tension_sensor.differential_amplifier, os->differential_amplifier);
    actual_tension = TAS501_tension_kg(&aw->tension_sensor.strain_gauge,  differential_amplifier_mV);
    aw->tension_sensor.tension_kg = actual_tension;
    
    
    
    motor_set_speed(os->modbus_drum, os->modbus_spooler, &aw->motor_state,2000);
    
    
    
    
    
#if 0
    os_gpio_write(aw->os.Endo70V_toggle,1);
    os_gpio_write(aw->os.Endo130V_toggle,1);
    os_delay_ms(2000);
    os_gpio_write(aw->os.Endo70V_toggle,0);
    os_gpio_write(aw->os.Endo130V_toggle,0);
    os_delay_ms(2000);
#endif
    
    r32 encoder_turns = get_encoder_turn_count(aw->os.spooler_encoder, aw->spooler_encoder_ppr);
    aw->cable_unwind_length_m += encoder_turns * aw->spooler_circumference_m;
    aw->cable_unwind_speed_mps = encoder_turns * aw->spooler_circumference_m / aw->control_system_dt;
    
    
}


#if 0

// TODO(ARUN): @intenal_timer_speed_test



void
HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	if(huart == os_uart_handle(aw->os.app_serial))
    {
        aw->comms_rx_count += 1;
        comms_parse_rx_and_create_tx(&aw->comms_rx, &aw->comms_tx);
        os_uart_send_dma(aw->os.app_serial, (u8 *)(&aw->comms_tx), aw->comms_tx.header.size);
        
	}
}

void 
HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == os_uart_handle(aw->os.app_serial))
    {
		aw->comms_tx_count += 1;
        MemoryZeroStruct(&aw->comms_rx);
        os_uart_receive_to_idle_dma(aw->os.app_serial, (u8 *)(&aw->comms_rx), sizeof(aw->comms_rx)*2);
        
    }
}
#endif





typedef enum Control_Mode Control_Mode;
enum Control_Mode
{
    Control_Mode_Manual,
    Control_Mode_Auto,
};

typedef enum Toggle_Mode Toggle_Mode;
enum Toggle_Mode
{
	Toggle_Mode_Low,
	Toggle_Mode_Mid,
	Toggle_Mode_High,
};


typedef struct Com_Control Com_Control;
struct Com_Control
{
    b32 drum_com_toggle;
    b32 spooler_com_toggle;
}; 


typedef struct Endo_Power_Control Endo_Power_Control;
struct Endo_Power_Control
{
    b32 Endo70V_toggle;
    b32 Endo130V_toggle;
}; 


typedef struct Panel_State Panel_State;
struct Panel_State
{
	Control_Mode control_mode;
	Toggle_Mode toggle_mode;
	i32 direction;
};

typedef struct Motor_State Motor_State;
struct Motor_State
{
	b32 activated;
	i32 drum_speed;
	i32 spooler_speed;
};

//
//- rsb: os state
//

enum
{
	Button_Control_Mode = 0,\
	Button_CW_Dir,
	Button_CCW_Dir,
	Button_Low_Speed,
	Button_High_Speed,
	Button_COUNT
};



typedef struct OS_State OS_State;
struct OS_State
{
	OS_Gpio *panel_buttons;
    OS_Gpio *drum_com_toggle;
    OS_Gpio *spooler_com_toggle;
    OS_Gpio *Endo70V_toggle;
    OS_Gpio *Endo130V_toggle;
	OS_Timer *spooler_encoder;
	OS_Timer_It *control_system;
	OS_I2c *differential_amplifier;
	OS_Uart *modbus_drum;
	OS_Uart *modbus_spooler;
    OS_Uart *app_serial;
};

//
//- rsb: Main state
//

typedef struct Tension_Sensor Tension_Sensor;
struct Tension_Sensor
{
	TAS501 strain_gauge;
	ADS1115 differential_amplifier;
	r32 tension_kg;
};

typedef struct AW_State AW_State;
struct AW_State
{
	Arena *arena;
	OS_State os;
	
	//- rsb: state
	u32 panel_buttons[Button_COUNT];
    Com_Control com_control;
    Endo_Power_Control endo_power_control;
	Panel_State panel_state;
	Motor_State motor_state;
	
	//- rsb: winch cable details
	r32 cable_unwind_length_m;
	r32 cable_unwind_speed_mps;
	
	//- rsb: spooler
	r32 spooler_circumference_m;;
	r32 spooler_encoder_ppr;
	
	//- rsb: control system
	PID tension_controller;;
	PID_Gains low_tension_gains;
	PID_Gains mid_tension_gains;
	PID_Gains high_tension_gains;
	r32 control_system_dt;
	
	//- rsb: Tension sensor
	Tension_Sensor tension_sensor;
    
	//- rsb: Serial communication
	u32 comms_rx_count;
	u32 comms_tx_count;
	u32 comms_error_count;
	Comms_Arch comms_arch;
	Comms_Tx comms_tx;
	Comms_Rx comms_rx;
	Comms_Cable_State state;
    
    
};

global_variable AW_State *aw = {0};
global_variable Arena *thread_arena[2] = {0};















#endif //C_D_H
