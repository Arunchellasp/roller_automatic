/* date = September 2nd 2025 11:58 am */



internal void
aw_init()
{
	//- rsb: sub-system boot time
	os_delay_ms(3000);
	
	//- rsb: thread arena init
	{
		for(u32 it = 0; it < ArrayCount(thread_arena); it += 1)
		{
			thread_arena[it] = arena_alloc(KB(1));
		}
	}
	
	//- rsb: arena init
	Arena *arena = arena_alloc(KB(10));
	aw = push_array(arena, AW_State, 1);
	aw->arena = arena;
	
	//- rsb: buttons init
	aw->os.panel_buttons = push_array(arena, OS_Gpio, Button_COUNT);
	os_gpio_init(aw->os.panel_buttons + Button_Control_Mode, stm32_gpio(MODE_IN));
	os_gpio_init(aw->os.panel_buttons + Button_CW_Dir,       stm32_gpio(CW_IN));
	os_gpio_init(aw->os.panel_buttons + Button_CCW_Dir,      stm32_gpio(CCW_IN));
	os_gpio_init(aw->os.panel_buttons + Button_Low_Speed,    stm32_gpio(SPEED_LOW_IN));
	os_gpio_init(aw->os.panel_buttons + Button_High_Speed,   stm32_gpio(SPEED_HIGH_IN));
	
    // NOTE(ARUN):  @com_controll_init
    aw->os.drum_com_toggle= push_array(arena, OS_Gpio, 1);
    os_gpio_init(aw->os.drum_com_toggle,stm32_gpio(DM_EN));
    aw->os.spooler_com_toggle = push_array(arena, OS_Gpio, 1);
    os_gpio_init(aw->os.spooler_com_toggle,stm32_gpio(SP_EN));
    
	//- rsb: comms init
	aw->os.modbus_drum = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.modbus_drum, &huart6, MODBUS_RESPONSE_TIMEOUT);
	aw->os.modbus_spooler = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.modbus_spooler, &huart3, MODBUS_RESPONSE_TIMEOUT);
    
	//- arun: app serial communication
    aw->os.app_serial = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.app_serial, &huart1, APP_COMMS_RESPONSE_TIMEOUT);
    comms_arch_init(&aw->comms_arch, APP_COMMS_SUPPORTED_ARCHITECURE, APP_COMMS_SUPPORTED_ARCHITECURE_VERSION);
	os_uart_receive_to_idle_dma(aw->os.app_serial, (u8 *)(&aw->comms_rx), sizeof(aw->comms_rx));
    
	//- rsb: tension sensor init
	ADS1115_init(&aw->tension_sensor.differential_amplifier, ADS1115_ADDR_GND, 0, ADS1115_GAIN_SIXTEEN, 0.0078125, ADS1115_CONVERSIONDELAY);
	aw->os.differential_amplifier = push_array(arena, OS_I2c, 1);
	os_i2c_init(aw->os.differential_amplifier, &hi2c1, 10);
	TAS501_init(&aw->tension_sensor.strain_gauge, 5.0f, 100.0f, 2.0f);
	
	//- rsb: encoder init
	aw->spooler_encoder_ppr = 4000.0f;
	aw->spooler_circumference_m = PI * 0.05f;
	aw->os.spooler_encoder = push_array(arena, OS_Timer, 1);
	os_timer_init(aw->os.spooler_encoder, &htim3, stm32_timer_encoder);
	os_timer_start(aw->os.spooler_encoder);
	
	//- rsb: control system init
	r32 timer_dt = 1.0f / 100.0f;
	aw->control_system_dt = timer_dt;
    
	aw->os.control_system = push_array(arena, OS_Timer_It, 1);
#define MAX_MOTOR_SPEED 3000.0f
	//pid_gains_init(&aw->low_tension_gains, 0.05, 0.0f, 0.0f, timer_dt); 
	pid_zn_gains_init(&aw->low_tension_gains, 0.05, 10.0f, timer_dt);
	//pid_gains_init(&aw->mid_tension_gains, 0.05, 0.0f, 0.0f, timer_dt); 
	pid_zn_gains_init(&aw->mid_tension_gains, 0.05, 10.0f, timer_dt);
	//pid_gains_init(&aw->high_tension_gains, 0.05, 0.0f, 0.0f, timer_dt); 
	pid_zn_gains_init(&aw->high_tension_gains, 0.05, 10.0f, timer_dt);
	pid_init(&aw->tension_controller, &aw->low_tension_gains, -1.0f, 1.0f, MAX_MOTOR_SPEED);
    
    
    os_gpio_write(aw->os.drum_com_toggle,1);
    os_gpio_write(aw->os.spooler_com_toggle,1);
    
    
    motor_activate(aw->os.modbus_drum, aw->os.modbus_spooler, &aw->motor_state, 1);
    
    os_gpio_write(aw->os.drum_com_toggle,0);
    os_gpio_write(aw->os.spooler_com_toggle,0);
    
    
	os_delay_ms(10);
	os_timer_it_init(aw->os.control_system ,&htim6, stm32_timer_it);
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
    
#if 0    
    // NOTE(ARUN): array read
    
    for(u32 it = 0; it < ArrayCount(aw->panel_buttons); it += 1)
    {
        aw->panel_buttons[it] = os_gpio_read(aw->os.panel_buttons + it);
    }
    
    
    // NOTE(ARUN): dual pole data collection
    
    Control_Mode control_mode = (aw->panel_buttons[Button_Control_Mode] == 0) ? Control_Mode_Auto :Control_Mode_Manual;
    aw->panel_state.control_mode = control_mode;
    
    
    // NOTE(ARUN): three polle trigger collection 
    
    Toggle_Mode toggle_mode = Toggle_Mode_Low;
    if(aw->panel_buttons[Button_Low_Speed] && aw->panel_buttons[Button_High_Speed])
    {
        toggle_mode = Toggle_Mode_Mid;
    }
    else if(aw->panel_buttons[Button_Low_Speed])
    {
        toggle_mode = Toggle_Mode_Low;
    }
    else if(aw->panel_buttons[Button_High_Speed])
    {
        toggle_mode = Toggle_Mode_High;
    } 
    
    aw->panel_state.toggle_mode = toggle_mode;
    
    
    // NOTE(ARUN): drection Data collection
    
    i32 direction = 0;
    if(aw->panel_buttons[Button_CCW_Dir] && aw->panel_buttons[Button_CW_Dir])
    {
        // NOTE(ARUN): Do nothing
    }
    else if(aw->panel_buttons[Button_CW_Dir])
    {
        direction = 1;
    }
    else if(aw->panel_buttons[Button_CCW_Dir])
    {
        direction = -1;
    }
    else
    {
        // NOTE(ARUN): Do nothing
    }
    aw->panel_state.direction = direction;
    
#endif
    
    // NOTE(ARUN): load_cell data collection
    
	r32 actual_tension = {0};
    r32 differential_amplifier_mV =
        ADS1115_read_adc_diff_23(&aw->tension_sensor.differential_amplifier, os->differential_amplifier);
    actual_tension = TAS501_tension_kg(&aw->tension_sensor.strain_gauge,  differential_amplifier_mV);
    aw->tension_sensor.tension_kg = actual_tension;
    
    
    // NOTE(ARUN): encoder cable counting
    
#if 0
    r32 encoder_turns = get_encoder_turn_count(aw->os.spooler_encoder, aw->spooler_encoder_ppr);
    aw->cable_unwind_length_m += encoder_turns * aw->spooler_circumference_m;
    aw->cable_unwind_speed_mps = encoder_turns * aw->spooler_circumference_m / aw->control_system_dt;
#endif
    
    // NOTE(ARUN):PID calculation
    
    
#if 0    
    i32 drum_speed = {0};
    r32 target_tension = {0};
    target_tension = 1.0f;
    pid_update_gains(&aw->tension_controller, &aw->low_tension_gains);
    drum_speed = (i32)pid_step(&aw->tension_controller, target_tension, actual_tension);
#endif
    
    
    
#if 0    
    os_gpio_write(aw->os.drum_com_toggle,1);
    os_gpio_write(aw->os.spooler_com_toggle,1);
    
    motor_set_speed(os->modbus_drum, os->modbus_spooler, &aw->motor_state, drum_speed);
    
    os_gpio_write(aw->os.drum_com_toggle,0);
    os_gpio_write(aw->os.spooler_com_toggle,0);
#endif
    
}



//
//- rsb: Timer IT callbacks
//


#if 0
void 
HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    OS_State *os = &aw->os;
    if(htim == os_timer_it_handle(os->control_system))
    {
        
    }
    
}
#endif




//
//- rsb: Communication callbacks
//

void 
HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
	if(huart == os_uart_handle(aw->os.app_serial))
    {
		if(HAL_UARTEx_GetRxEventType(huart) == HAL_UART_RXEVENT_TC)
		{
			aw->comms_rx_count += 1;
			comms_parse_rx_and_create_tx(&aw->comms_rx, &aw->comms_tx);
			os_uart_send_dma(aw->os.app_serial, (u8 *)(&aw->comms_tx), aw->comms_tx.header.size);
        }
	}
}

void 
HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == os_uart_handle(aw->os.app_serial))
    {
		aw->comms_tx_count += 1;
        MemoryZeroStruct(&aw->comms_rx);
        os_uart_receive_to_idle_dma(aw->os.app_serial, (u8 *)(&aw->comms_rx), sizeof(aw->comms_rx));
    }
}

void
HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart == os_uart_handle(aw->os.app_serial))
    {
        aw->comms_error_count += 1;
        HAL_UART_AbortReceive(huart);
        MemoryZeroStruct(&aw->comms_rx);
        os_uart_receive_to_idle_dma(aw->os.app_serial, (u8 *)(&aw->comms_rx), sizeof(aw->comms_rx));
    }
}



#endif //