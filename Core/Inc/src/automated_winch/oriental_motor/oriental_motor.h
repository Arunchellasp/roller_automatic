
#ifndef ORIENTAL_MOTOR_H
#define ORIENTAL_MOTOR_H

// NOTE(ARUN): Motor Parameters
#define MOTOR_CONTINUOUS_OPERATION_SPEED_CONTROL  0x0030
#define MOTOR_OPERATION_MODE MOTOR_CONTINUOUS_OPERATION_SPEED_CONTROL
#define MOTOR_ACCELERATION_LIMIT    5    // ms
#define MOTOR_DECELERATION_LIMIT    5    // ms
#define MOTOR_TORQUE_LIMIT          900     // 25.00%

// NOTE(ARUN): Modbus Parameters
#define MODBUS_CMD_DELAY_MS 8
#define MODBUS_RESPONSE_TIMEOUT 1000

//char msg[] = "Hello World\r\n";

//
//~ NOTE(ARUN): Motor Control Functions Implementation
//

internal inline
u16 modbus_crc(u8 *buffer, u16 length)
{
    u16 crc = 0xFFFF;
    for (u16 i = 0; i < length; i++)
	{
        crc ^= buffer[i];
        for (u8 j = 0; j < 8; j++)
		{
            if(crc & 0x0001)
			{
                crc = (crc >> 1) ^ 0xA001;
            }
			else
			{
                crc >>= 1;
            }
        }
    }
    return crc;
}

internal b32
modbus_write_multiple_registers(OS_Uart *uart, u8 id, u16 register_address, u32 *data, u8 count)
{
	//- rsb: data structures
#pragma pack(push, 1)
	typedef struct Modbus_Packet_Header Modbus_Packet_Header;
	struct Modbus_Packet_Header
	{
		u8 id;
		u8 function_code;
		u16 register_address;
		u16 register_count;
	};
#pragma pack(pop)
    
	//- rsb: inits
	Temp temp = temp_begin(thread_arena[0]);
	Assert((sizeof(*data) * count) <= 255);
	u8 data_size = sizeof(*data) * count;
	u32 packet_size = sizeof(Modbus_Packet_Header) + 1 + data_size + 2;
	u8 *packet = push_array(temp.arena, u8, packet_size);
	u32 pos = 0;
    
	//- rsb: write header
	{
		Modbus_Packet_Header *header = (Modbus_Packet_Header *)packet;
		header->id = id;
		header->function_code = 0x10;
		header->register_address = __REV16(register_address);
		u16 register_count = sizeof(register_count) * count;
		header->register_count = __REV16(register_count);
		pos += sizeof(Modbus_Packet_Header);
	}
    
	//- rsb: write bytes
	packet[pos] = data_size;
	pos += 1;
    
	//- rsb: write data
	for(u8 it = 0; it < count; it += 1)
	{
		*(u32 *)(packet + pos) = __REV(data[it]);
		pos += sizeof(*data);
	}
    
	//- rsb: write crc
	u16 crc = modbus_crc(packet, pos);
	*(u16 *)(packet + pos) = crc; pos += 2;
	Assert(pos == packet_size);
    
	//- rsb: send packet
	b32 result = os_uart_send(uart, packet, packet_size);
    
	temp_end(temp);
	return result;
}

internal b32
motor_excitation(OS_Uart *uart, u8 id, u32 on)
{
#define MODBUS_REG_EXCITE 0x007C
	u32 data[1] = {(on == 0) ? 0 : 1};
	b32 result = modbus_write_multiple_registers(uart, id, MODBUS_REG_EXCITE, data, ArrayCount(data));
	return result;
}

internal b32
motor_speed_control(OS_Uart *uart, u8 id, i32 speed)
{
#define MODBUS_REG_OPERATION_MODE 0x005A
	u32 data[7] = {0};
	data[0] = MOTOR_OPERATION_MODE;
	data[1] = 0;
	data[2] = (u32)speed;
	data[3] = MOTOR_ACCELERATION_LIMIT;
	data[4] = MOTOR_DECELERATION_LIMIT;
	data[5] = MOTOR_TORQUE_LIMIT;
	data[6] = 1;
	b32 result = modbus_write_multiple_registers(uart, id, MODBUS_REG_OPERATION_MODE, data, ArrayCount(data));
	return result;
}

#define Drum_ID 1
#define Spooler_ID 2
#define Spooler_Speed_Reduction 3.4
internal b32 
motor_activate(OS_Uart *uart_drum, OS_Uart *uart_spooler, Motor_State *motor, b32 activate_motor)
{
	b32 result = 1;
	result &= motor_excitation(uart_drum, Drum_ID, activate_motor);
	result &= motor_excitation(uart_spooler, Spooler_ID, activate_motor);
	motor->activated = activate_motor;
	return result;
}

internal b32 
motor_set_speed(OS_Uart *uart_drum, OS_Uart *uart_spooler, Motor_State *motor, i32 drum_speed)
{
	b32 result = 1;
	motor->drum_speed = drum_speed;
	result &= motor_speed_control(uart_drum, Drum_ID, motor->drum_speed);
	//result &= motor_speed_control(uart_drum, Drum_ID, -800);
    motor->spooler_speed = drum_speed / Spooler_Speed_Reduction;
    result &= motor_speed_control(uart_spooler, Spooler_ID, motor->spooler_speed);
    //result &= motor_speed_control(uart_spooler, Spooler_ID, -800);
    
    
    return result;
}

#endif //ORIENTAL_MOTOR_H
