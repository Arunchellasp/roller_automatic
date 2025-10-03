
internal void
aw_init()
{
    // NOTE(ARUN):@sub_system_boot_time
	os_delay_ms(3000);
	
    // NOTE(ARUN):@thread_arena_init
	{
		for(u32 it = 0; it < ArrayCount(thread_arena); it += 1)
		{
			thread_arena[it] = arena_alloc(KB(1));
		}
	}
	
	// NOTE(ARUN): @arena_init
	Arena *arena = arena_alloc(KB(10));
	aw = push_array(arena, AW_State, 1);
	aw->arena = arena;
	
    
	// NOTE(ARUN): @panel_buttons_init
	aw->os.panel_buttons = push_array(arena, OS_Gpio, Button_COUNT);
	os_gpio_init(aw->os.panel_buttons + Button_Control_Mode, stm32_gpio(MODE_IN));
	os_gpio_init(aw->os.panel_buttons + Button_CW_Dir,       stm32_gpio(CW_IN));
	os_gpio_init(aw->os.panel_buttons + Button_CCW_Dir,      stm32_gpio(CCW_IN));
	os_gpio_init(aw->os.panel_buttons + Button_Low_Speed,    stm32_gpio(SPEED_LOW_IN));
	os_gpio_init(aw->os.panel_buttons + Button_High_Speed,   stm32_gpio(SPEED_HIGH_IN));
	
    
    // NOTE(ARUN):  @motorS_serial_enable_init
	aw->os.drum_com_toggle= push_array(arena, OS_Gpio, 1);
	os_gpio_init(aw->os.drum_com_toggle,stm32_gpio(EN_DRUM_COM));
	aw->os.spooler_com_toggle = push_array(arena, OS_Gpio, 1);
	os_gpio_init(aw->os.spooler_com_toggle,stm32_gpio(EN_SPOOLER_COM));
    
    
    // NOTE(ARUN):  @motorS_serial_enable_init
	aw->os.ac_relay_toggle= push_array(arena, OS_Gpio, 1);
	os_gpio_init(aw->os.ac_relay_toggle,stm32_gpio(AC_RELAY));
	aw->os.dc_relay_toggle = push_array(arena, OS_Gpio, 1);
	os_gpio_init(aw->os.dc_relay_toggle,stm32_gpio(DC_RELAY));
    
	// NOTE(ARUN): @motors_serial_init
	aw->os.modbus_drum = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.modbus_drum, &huart6, MODBUS_RESPONSE_TIMEOUT);
	aw->os.modbus_spooler = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.modbus_spooler, &huart3, MODBUS_RESPONSE_TIMEOUT);
    
	// NOTE(ARUN): @app_serial_communication
    aw->os.app_serial = push_array(arena, OS_Uart, 1);
	os_uart_init(aw->os.app_serial, &huart1, APP_COMMS_RESPONSE_TIMEOUT);
    comms_arch_init(&aw->comms_arch, APP_COMMS_SUPPORTED_ARCHITECURE, APP_COMMS_SUPPORTED_ARCHITECURE_VERSION);
	os_uart_receive_to_idle_dma(aw->os.app_serial, (u8 *)(&aw->comms_rx), sizeof(aw->comms_rx));
	
	// NOTE(ARUN): @tension_sensor_init
	ADS1115_init(&aw->tension_sensor.differential_amplifier, ADS1115_ADDR_GND, 0, ADS1115_GAIN_SIXTEEN, 0.0078125, ADS1115_CONVERSIONDELAY);
	aw->os.differential_amplifier = push_array(arena, OS_I2c, 1);
	os_i2c_init(aw->os.differential_amplifier, &hi2c1, 10);
	TAS501_init(&aw->tension_sensor.strain_gauge, 5.0f, 200.0f, 2.0f);
	
	// NOTE(ARUN): @encoder_init
    
	aw->spooler_encoder_ppr = 4000.0f;
	aw->spooler_circumference_m = PI * 0.05f;
	aw->os.spooler_encoder = push_array(arena, OS_Timer, 1);
	os_timer_init(aw->os.spooler_encoder, &htim3, stm32_timer_encoder);
	os_timer_start(aw->os.spooler_encoder);
    
    // NOTE(ARUN): @internal_timer
	// NOTE(ARUN): @control_system_init
	r32 timer_dt = 1.0f / 100.0f;
	aw->control_system_dt = timer_dt;
	aw->os.control_system = push_array(arena, OS_Timer_It, 1);
#define MAX_MOTOR_SPEED 2000.0f
    //pid_gains_init(&aw->low_tension_gains, 0.051, 0.0f, 0.0f, timer_1dt); 
#if 1
	pid_zn_gains_init(&aw->low_tension_gains, 0.052, 10.0f, timer_dt);
	//pid_gains_init(&aw->mid_tension_gains, 0.05, 0.0f, 0.0f, timer_dt); 
	pid_zn_gains_init(&aw->mid_tension_gains, 0.052, 10.0f, timer_dt);
	//pid_gains_init(&aw->high_tension_gains, 0.05, 0.0f, 0.0f, timer_dt); 
	pid_zn_gains_init(&aw->high_tension_gains, 0.052, 10.0f, timer_dt);
#endif
    pid_init(&aw->tension_controller, &aw->low_tension_gains, -1.0f, 1.0f, MAX_MOTOR_SPEED);
    
    // NOTE(ARUN):@enable_motor_exitation_and_motors_serial_enable 
    os_gpio_write(aw->os.drum_com_toggle,1);
    os_gpio_write(aw->os.spooler_com_toggle,1);
	motor_activate(aw->os.modbus_drum, aw->os.modbus_spooler, &aw->motor_state, 1);
	os_delay_ms(10);
	os_timer_it_init(aw->os.control_system ,&htim6, stm32_timer_it);
	os_timer_it_start(aw->os.control_system);
    
    // NOTE(ARUN): @Endo_power_swtching
    os_gpio_write(aw->os.ac_relay_toggle,1);
    os_gpio_write(aw->os.dc_relay_toggle,1);
    
    
}



// NOTE(ARUN): @encoder_trun_count

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
    
}





// NOTE(ARUN): @Timer_IT_callbacks 

void 
HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	OS_State *os = &aw->os;
	if(htim == os_timer_it_handle(os->control_system))
    {
        
        // NOTE(ARUN): @update_panale_button_state
		for(u32 it = 0; it < ArrayCount(aw->panel_buttons); it += 1)
		{
			aw->panel_buttons[it] = os_gpio_read(aw->os.panel_buttons + it);
		}
        
		
        // NOTE(ARUN):@get_control_mode 
		Control_Mode control_mode = (aw->panel_buttons[Button_Control_Mode] == 1) ? Control_Mode_Auto : Control_Mode_Manual;
		aw->panel_state.control_mode = control_mode;
		
        
		// NOTE(ARUN)@get_toggle_mode
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
		
		// NOTE(ARUN): @get_direction
        
		i32 direction = 0;
		if(aw->panel_buttons[Button_CCW_Dir] && aw->panel_buttons[Button_CW_Dir])
		{
			// NOTE(ARUN):do nothing
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
            // NOTE(ARUN): do nothing
		}
		aw->panel_state.direction = direction;
		
        
        
		// NOTE(ARUN): @calc_and_set_drum_speed
        
		i32 drum_speed = {0};
        
		switch(control_mode)
		{
			case Control_Mode_Manual:
			{
				switch(toggle_mode)
				{
					case Toggle_Mode_Low: drum_speed = 800; break;
					case Toggle_Mode_Mid: drum_speed = 2000; break;
					case Toggle_Mode_High: drum_speed = 3000; break;
				}
                
                
                r32 actual_tension = {0};
                
                r32 differential_amplifier_mV = ADS1115_read_adc_diff_01(&aw->tension_sensor.differential_amplifier, os->differential_amplifier);
                actual_tension = TAS501_tension_kg(&aw->tension_sensor.strain_gauge,  differential_amplifier_mV);
                aw->tension_sensor.tension_kg = actual_tension;
                
                
                
                
                
				drum_speed *= direction;
			}break;
			
			case Control_Mode_Auto:
			{
                
				// NOTE(ARUN): @tension_sensor_read
                
				r32 actual_tension = {0};
				{
					r32 differential_amplifier_mV = ADS1115_read_adc_diff_01(&aw->tension_sensor.differential_amplifier, os->differential_amplifier);
					actual_tension = TAS501_tension_kg(&aw->tension_sensor.strain_gauge,  differential_amplifier_mV);
					aw->tension_sensor.tension_kg = actual_tension;
				}
				
				// NOTE(ARUN): @get_target_tension
                
				r32 target_tension = {0};
				switch(toggle_mode)
				{
					case Toggle_Mode_Low: 
					{
						target_tension = 7.5f;
						pid_update_gains(&aw->tension_controller, &aw->low_tension_gains);
					}break;
					case Toggle_Mode_Mid: 
					{
						target_tension = 7.5f;
						pid_update_gains(&aw->tension_controller, &aw->mid_tension_gains);
					}break;
					case Toggle_Mode_High: 
					{
						target_tension = 3.75f;
						pid_update_gains(&aw->tension_controller, &aw->high_tension_gains);
					}break;
				}
				
				// NOTE(ARUN): @pid_speed_calculation
                
				drum_speed = (i32)pid_step(&aw->tension_controller, target_tension, actual_tension);
			}break;
		}
		motor_set_speed(os->modbus_drum, os->modbus_spooler, &aw->motor_state, drum_speed);
		
        
        r32 encoder_turns = get_encoder_turn_count(aw->os.spooler_encoder, aw->spooler_encoder_ppr);
		aw->cable_unwind_length_m += encoder_turns * aw->spooler_circumference_m;
		aw->cable_unwind_speed_mps = encoder_turns * aw->spooler_circumference_m / aw->control_system_dt;
        
	}
}



// NOTE(ARUN): @Communication_callbacks



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

