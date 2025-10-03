

#ifndef AUTOMATED_WINCH_H
#define AUTOMATED_WINCH_H


//- rsb: button, motor, panel state


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

typedef struct Com_Control Com_Control;
struct Com_Control
{
    b32 drum_com_toggle;
    b32 spooler_com_toggle;
}; 


//- rsb: os state


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
    OS_Gpio *ac_relay_toggle;
    OS_Gpio *dc_relay_toggle;
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
};

global_variable AW_State *aw = {0};
global_variable Arena *thread_arena[2] = {0};

#endif //AUTOMATED_WINCH_MAIN_H