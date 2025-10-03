
internal void
comms_arch_init(Comms_Arch *arch, Comms_Arch_Type type, u32 version)
{
	MemoryZeroStruct(arch);
	arch->type = type;
	arch->version = version;
}

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
		}
	}
	header->checksum = checksum;
	return result;
}

internal b32 
aw_comms_arch_verify(Comms_Rx *rx)
{
	Comms_Arch *base = &aw->comms_arch;
	Comms_Arch *rcvd = &rx->arch_v1.architecture;
	b32 result = (base->type == rcvd->type) && (base->version == rcvd->version);
	base->is_verified = result;
	return result;
}

internal b32
aw_comms_arch_is_verified()
{
	b32 result = aw->comms_arch.is_verified;
	return result;
}

internal Comms_Cable_State
aw_comms_get_cable_state()
{
	Comms_Cable_State result = {0};
	result.unwind_length_m = aw->cable_unwind_length_m;
	result.unwind_speed_mps = aw->cable_unwind_speed_mps;
	result.tension_kg = aw->tension_sensor.tension_kg;
	return result;
}

#define comms_create_packet_header(packet_type, packet_version) (Comms_Packet_Header)\
{ \
.type = Comms_Packet_Type_##packet_type, \
.version = packet_version, \
.size = sizeof(Comms_Packet_##packet_type##_v##packet_version), \
.checksum = 0 \
}

internal void
comms_create_packet_arch_v1(Comms_Tx *tx, b32 is_verified)
{
	MemoryZeroStruct(tx);
	tx->arch_v1.header = comms_create_packet_header(Architecture, 1);
	tx->arch_v1.architecture.type = APP_COMMS_SUPPORTED_ARCHITECURE;
	tx->arch_v1.architecture.version = APP_COMMS_SUPPORTED_ARCHITECURE_VERSION;
	tx->arch_v1.architecture.is_verified = is_verified;
	tx->header.checksum = sdbm_checksum((u8 *)tx, tx->header.size);
}

internal void
comms_create_packet_state_v1(Comms_Tx *tx, Comms_Cable_State cable_state)
{
	MemoryZeroStruct(tx);
	tx->state_v1.header = comms_create_packet_header(State, 1);
	tx->state_v1.cable = cable_state;
	tx->header.checksum = sdbm_checksum((u8 *)tx, tx->header.size);
}

internal void
comms_create_packet_error_v1(Comms_Tx *tx, Comms_Error error)
{
	MemoryZeroStruct(tx);
	tx->error_v1.header = comms_create_packet_header(Error, 1);
	tx->error_v1.error = error;
	tx->header.checksum = sdbm_checksum((u8 *)tx, tx->header.size);
}

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
		
		case Comms_Packet_Type_Error:
		default:
		comms_create_packet_error_v1(tx, Comm_Error_Invalid_Packet_ID);
		goto end;
	};
	
	end:
	return;
}