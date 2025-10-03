#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#define APP_COMMS_RESPONSE_TIMEOUT 50 
#define APP_COMMS_SUPPORTED_ARCHITECURE Comms_Arch_Type_Automated_Winch 
#define APP_COMMS_SUPPORTED_ARCHITECURE_VERSION 1


typedef u32 Comms_Packet_Type;
enum 
{
    Comms_Packet_Type_Architecture ='ARCH',
    Comms_Packet_Type_State = 'STAT',
    Comms_Packet_Type_Control = 'CONT',
    Comms_Packet_Type_Error = 'ERRS',
};

typedef struct Comms_Packet_Header Comms_Packet_Header;
struct Comms_Packet_Header
{
    Comms_Packet_Type type;
    u32 version;
    u32 size;     //- rsb: Size of whole packet incl. header
    u32 checksum; //- rsb: SDBM checksum of whole packet incl. header
};

//
//- rsb: ARCH
//

typedef u32 Comms_Arch_Type;
enum
{
    Comms_Arch_Type_Unidentified = 'NOID',
    Comms_Arch_Type_Automated_Winch = 
        'ATWN',
};

typedef struct Comms_Arch Comms_Arch;
struct Comms_Arch
{
    Comms_Arch_Type type;
    u32 version;
    union
    {
        u32 is_supported;
        u32 is_verified;
    };
};

typedef struct Comms_Packet_Architecture_v1 Comms_Packet_Architecture_v1;
struct Comms_Packet_Architecture_v1
{
    Comms_Packet_Header header;
    Comms_Arch architecture;
};

//
//- rsb: STAT
//


typedef struct Comms_Cable_State Comms_Cable_State;
struct Comms_Cable_State
{
	r32 unwind_length_m;
	r32 unwind_speed_mps;
	r32 tension_kg;
    r32 type;
    r32 control;
	r32 selection;
	r32 speed;
};

typedef struct Comms_Packet_State_v1 Comms_Packet_State_v1;
struct Comms_Packet_State_v1
{
    Comms_Packet_Header header;
	Comms_Cable_State cable;
    
    
};



typedef struct Comms_Winch_control Comms_Winch_control;
struct Comms_Winch_control
{
    b32 hmi_control;
    
#if 0
    r32 control;
	r32 selection;
	r32 speed;
#endif
};

typedef struct Comms_Packet_Control_v1 Comms_Packet_Control_v1;
struct Comms_Packet_Control_v1
{
    Comms_Packet_Header header;
    Comms_Winch_control control;
    
};

//
//- rsb: ERRS
//

typedef u32 Comms_Error;
enum
{
	Comm_Error_Packet_Corrupted        = (1 << 0),
	Comm_Error_Unverified_Architecture = (1 << 1),
	Comm_Error_Invalid_Packet_ID       = (1 << 2),
	Comm_Error_Invalid_Packet_Version  = (1 << 3),
};

typedef struct Comms_Packet_Error_v1 Comms_Packet_Error_v1;
struct Comms_Packet_Error_v1
{
    Comms_Packet_Header header;
	Comms_Error error;
};

//
//- rsb: Tx/Rx
//

typedef struct Comms_Tx Comms_Tx;
struct Comms_Tx
{
	union
	{
		Comms_Packet_Header header;
		Comms_Packet_Architecture_v1 arch_v1;
		Comms_Packet_State_v1 state_v1;
        //Comms_Packet_Control_v1 control_v1;
		Comms_Packet_Error_v1 error_v1;
	};
};

typedef struct Comms_Rx Comms_Rx;
struct Comms_Rx
{
	union
	{
		Comms_Packet_Header header;
        Comms_Packet_Architecture_v1 arch_v1;
		Comms_Packet_State_v1 state_v1;
        //Comms_Packet_Control_v1 control_v1;
	};
};

#endif //SERIAL_COMMUNICATION_H
