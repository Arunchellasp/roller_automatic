/* date = September 10th 2025 5:01 pm */

#ifndef TEST_H
#define TEST_H

//
//- Node 1 to Node 2: Add a new Control packet type
//


// Add to packet types enum
typedef u32 Comms_Packet_Type;
enum 
{
    Comms_Packet_Type_Architecture = 'ARCH',
    Comms_Packet_Type_State = 'STAT',
    Comms_Packet_Type_Control = 'CTRL',  // NEW: Control commands from node 1 to node 2
    Comms_Packet_Type_Error = 'ERRS',
};


// NEW: Control data structure (3 data items from node 1 to node 2)
typedef struct Comms_Control_Command Comms_Control_Command;
struct Comms_Control_Command
{
    r32 target_speed_mps;        // Target cable speed
    r32 target_position_m;       // Target cable position
    u32 operation_mode;          // Operation mode (0=manual, 1=auto, 2=emergency_stop, etc.)
};

typedef struct Comms_Packet_Control_v1 Comms_Packet_Control_v1;
struct Comms_Packet_Control_v1
{
    Comms_Packet_Header header;
    Comms_Control_Command control;
};

//
//- Node 2 to Node 1: Enhance State packet (4 data items)
//

// ENHANCED: Cable state structure (4 data items from node 2 to node 1)
typedef struct Comms_Cable_State Comms_Cable_State;
struct Comms_Cable_State
{
    r32 unwind_length_m;         // Current cable unwind length
    r32 unwind_speed_mps;        // Current cable unwind speed
    r32 tension_kg;              // Current cable tension
    u32 system_status;           // NEW: System status flags (ready, error, emergency, etc.)
};

//
//- Updated Tx/Rx structures
//

typedef struct Comms_Tx Comms_Tx;
struct Comms_Tx
{
    union
    {
        Comms_Packet_Header header;
        Comms_Packet_Architecture_v1 arch_v1;
        Comms_Packet_State_v1 state_v1;
        Comms_Packet_Control_v1 control_v1;    // NEW
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
        Comms_Packet_Control_v1 control_v1;    // NEW
        Comms_Packet_Error_v1 error_v1;        // Add error to Rx for completeness
    };
};

//
//- New functions for Node 1 (sending control commands)
//

internal Comms_Control_Command
node1_get_control_command()
{
    Comms_Control_Command result = {0};
    // Get control parameters from node 1's application logic
    result.target_speed_mps = node1->target_speed;
    result.target_position_m = node1->target_position;
    result.operation_mode = node1->operation_mode;
    return result;
}

internal void
comms_create_packet_control_v1(Comms_Tx *tx, Comms_Control_Command control)
{
    MemoryZeroStruct(tx);
    tx->control_v1.header = comms_create_packet_header(Control, 1);
    tx->control_v1.control = control;
    tx->header.checksum = sdbm_checksum((u8 *)tx, tx->header.size);
}

//
//- Enhanced functions for Node 2 (enhanced state reporting)
//

internal Comms_Cable_State
aw_comms_get_cable_state()
{
    Comms_Cable_State result = {0};
    result.unwind_length_m = aw->cable_unwind_length_m;
    result.unwind_speed_mps = aw->cable_unwind_speed_mps;
    result.tension_kg = aw->tension_sensor.tension_kg;
    result.system_status = aw->system_status_flags;  // NEW: 4th data item
    return result;
}

//
//- Updated packet validation
//

internal b32
comms_packet_valid(Comms_Rx *rx)
{
    b32 result = 0;
    
    //- rsb: get checksum
    Comms_Packet_Header *header = &rx->header;
    u32 checksum = header->checksum;
    header->checksum = 0;
    
    //- rsb: calculate checksum
    u32 calculated_checksum = 0;
    {
        switch(header->type)
        {
            case Comms_Packet_Type_Error:
            default:
            break;
            
            case Comms_Packet_Type_Architecture:
            {
                if(header->version == 1)
                {
                    calculated_checksum = sdbm_checksum((u8 *)rx, sizeof(Comms_Packet_Architecture_v1));
                    result = checksum == calculated_checksum;
                }
            }break;
            
            case Comms_Packet_Type_State:
            {
                if(header->version == 1)
                {
                    calculated_checksum = sdbm_checksum((u8 *)rx, sizeof(Comms_Packet_State_v1));
                    result = checksum == calculated_checksum;
                }
            }break;
            
            case Comms_Packet_Type_Control:  // NEW
            {
                if(header->version == 1)
                {
                    calculated_checksum = sdbm_checksum((u8 *)rx, sizeof(Comms_Packet_Control_v1));
                    result = checksum == calculated_checksum;
                }
            }break;
        }
    }
    header->checksum = checksum;
    return result;
}

//
//- Updated packet parsing for Node 2 (to handle control commands from Node 1)
//

internal void
comms_parse_rx_and_create_tx(Comms_Rx *rx, Comms_Tx *tx)
{
    if(!comms_packet_valid(rx))
    {
        comms_create_packet_error_v1(tx, Comm_Error_Packet_Corrupted);
        goto end;
    }
    
    switch(rx->header.type)
    {
        case Comms_Packet_Type_Architecture:
        {
            if(rx->header.version == 1)
            {
                b32 verified = aw_comms_arch_verify(rx);
                comms_create_packet_arch_v1(tx, verified);
                goto end;
            }
            
            comms_create_packet_error_v1(tx, Comm_Error_Invalid_Packet_Version);
            goto end;
        }break;
        
        case Comms_Packet_Type_State:
        {
            if(!aw_comms_arch_is_verified())
            {
                comms_create_packet_error_v1(tx, Comm_Error_Unverified_Architecture);
                goto end;
            }
            
            if(rx->header.version == 1)
            {
                comms_create_packet_state_v1(tx, aw_comms_get_cable_state());
                goto end;
            }
            
            comms_create_packet_error_v1(tx, Comm_Error_Invalid_Packet_Version);
            goto end;
        }break;
        
        case Comms_Packet_Type_Control:  // NEW: Handle control commands from Node 1
        {
            if(!aw_comms_arch_is_verified())
            {
                comms_create_packet_error_v1(tx, Comm_Error_Unverified_Architecture);
                goto end;
            }
            
            if(rx->header.version == 1)
            {
                // Process the control command
                aw_process_control_command(&rx->control_v1.control);
                
                // Respond with current state (4 data items)
                comms_create_packet_state_v1(tx, aw_comms_get_cable_state());
                goto end;
            }
            
            comms_create_packet_error_v1(tx, Comm_Error_Invalid_Packet_Version);
            goto end;
        }break;
        
        case Comms_Packet_Type_Error:
        default:
        comms_create_packet_error_v1(tx, Comm_Error_Invalid_Packet_ID);
        goto end;
    };
    
    end:
    return;
}

//
//- New function for Node 2 to process control commands
//

internal void
aw_process_control_command(Comms_Control_Command *control)
{
    // Apply the control commands from Node 1
    aw->target_speed_mps = control->target_speed_mps;
    aw->target_position_m = control->target_position_m;
    aw->operation_mode = control->operation_mode;
    
    // Update system status based on the command
    switch(control->operation_mode)
    {
        case 0: // Manual mode
        aw->system_status_flags |= SYSTEM_STATUS_MANUAL_MODE;
        break;
        case 1: // Auto mode  
        aw->system_status_flags |= SYSTEM_STATUS_AUTO_MODE;
        break;
        case 2: // Emergency stop
        aw->system_status_flags |= SYSTEM_STATUS_EMERGENCY_STOP;
        aw->target_speed_mps = 0.0f; // Override speed to 0
        break;
    }
}
#endif //TEST_H
