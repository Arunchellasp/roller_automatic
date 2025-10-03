
#ifndef ADS1115_ADS1015_H
#define ADS1115_ADS1015_H

// NOTE(ARUN): I2C ADDRESS/BITS - Connect the following pin to ADDR
#define ADS1115_ADDR_GND (0x48 << 1) // 1001 000 (ADDR -> GND)
#define ADS1115_ADDR_VDD (0x49 << 1) // 1001 001 (ADDR -> VDD)
#define ADS1115_ADDR_SDA (0x4A << 1) // 1001 010 (ADDR -> SDA)
#define ADS1115_ADDR_SCL (0x4B << 1) // 1001 011 (ADDR -> SCL)

// NOTE(ARUN): CONVERSION DELAY (in mS)
#define ADS1015_CONVERSIONDELAY (1) // Conversion delay
#define ADS1115_CONVERSIONDELAY (8) // Conversion delay

// NOTE(ARUN): POINTER REGISTER
#define ADS1015_REG_POINTER_MASK         (0x03)   // Point mask
#define ADS1015_REG_POINTER_CONVERT      (0x00)   // Conversion
#define ADS1015_REG_POINTER_CONFIG       (0x01)   // Configuration
#define ADS1015_REG_POINTER_LOWTHRESH    (0x02)   // Low threshold
#define ADS1015_REG_POINTER_HITHRESH     (0x03)   // High threshold

// NOTE(ARUN): CONFIG REGISTER
#define ADS1015_REG_CONFIG_OS_MASK      (0x8000) // OS Mask
#define ADS1015_REG_CONFIG_OS_SINGLE    (0x8000) // Write: Set to start a single-conversion
#define ADS1015_REG_CONFIG_OS_BUSY      (0x0000) // Read: Bit = 0 when conversion is in progress
#define ADS1015_REG_CONFIG_OS_NOTBUSY   (0x8000) // Read: Bit = 1 when device is not performing a conversion

#define ADS1015_REG_CONFIG_MUX_MASK     (0x7000) // Mux Mask
#define ADS1015_REG_CONFIG_MUX_DIFF_0_1 (0x0000) // Differential P = AIN0, N = AIN1 (default)
#define ADS1015_REG_CONFIG_MUX_DIFF_0_3 (0x1000) // Differential P = AIN0, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_1_3 (0x2000) // Differential P = AIN1, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_2_3 (0x3000) // Differential P = AIN2, N = AIN3
#define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000) // Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000) // Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000) // Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000) // Single-ended AIN3

#define ADS1015_REG_CONFIG_PGA_MASK     (0x0E00) // PGA Mask
#define ADS1015_REG_CONFIG_PGA_6_144V   (0x0000) // +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V   (0x0200) // +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V   (0x0400) // +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V   (0x0600) // +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V   (0x0800) //  +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V   (0x0A00) // +/-0.256V range = Gain 16

#define ADS1015_REG_CONFIG_MODE_MASK    (0x0100) // Mode Mask
#define ADS1015_REG_CONFIG_MODE_CONTIN  (0x0000) // Continuous conversion mode
#define ADS1015_REG_CONFIG_MODE_SINGLE  (0x0100) // Power-down single-shot mode (default)

#define ADS1015_REG_CONFIG_DR_MASK      (0x00E0) // Data Rate Mask
#define ADS1015_REG_CONFIG_DR_128SPS    (0x0000) // 128 samples per second
#define ADS1015_REG_CONFIG_DR_250SPS    (0x0020) // 250 samples per second
#define ADS1015_REG_CONFIG_DR_490SPS    (0x0040) // 490 samples per second
#define ADS1015_REG_CONFIG_DR_920SPS    (0x0060) // 920 samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS   (0x0080) // 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS   (0x00A0) // 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS   (0x00C0) // 3300 samples per second

#define ADS1015_REG_CONFIG_CMODE_MASK   (0x0010) // CMode Mask
#define ADS1015_REG_CONFIG_CMODE_TRAD   (0x0000) // Traditional comparator with hysteresis (default)
#define ADS1015_REG_CONFIG_CMODE_WINDOW (0x0010) // Window comparator

#define ADS1015_REG_CONFIG_CPOL_MASK    (0x0008) // CPol Mask
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW (0x0000) // ALERT/RDY pin is low when active (default)
#define ADS1015_REG_CONFIG_CPOL_ACTVHI  (0x0008) // ALERT/RDY pin is high when active

#define ADS1015_REG_CONFIG_CLAT_MASK    (0x0004) // Determines if ALERT/RDY pin latches once asserted
#define ADS1015_REG_CONFIG_CLAT_NONLAT  (0x0000) // Non-latching comparator (default)
#define ADS1015_REG_CONFIG_CLAT_LATCH   (0x0004) // Latching comparator

#define ADS1015_REG_CONFIG_CQUE_MASK    (0x0003) // CQue Mask
#define ADS1015_REG_CONFIG_CQUE_1CONV   (0x0000) // Assert ALERT/RDY after one conversions
#define ADS1015_REG_CONFIG_CQUE_2CONV   (0x0001) // Assert ALERT/RDY after two conversions
#define ADS1015_REG_CONFIG_CQUE_4CONV   (0x0002) // Assert ALERT/RDY after four conversions
#define ADS1015_REG_CONFIG_CQUE_NONE    (0x0003) // Disable the comparator and put ALERT/RDY in high state (default)

#define ADS1115_GAIN_TWOTHIRDS   ADS1015_REG_CONFIG_PGA_6_144V
#define ADS1115_GAIN_ONE         ADS1015_REG_CONFIG_PGA_4_096V
#define ADS1115_GAIN_TWO         ADS1015_REG_CONFIG_PGA_2_048V
#define ADS1115_GAIN_FOUR        ADS1015_REG_CONFIG_PGA_1_024V
#define ADS1115_GAIN_EIGHT       ADS1015_REG_CONFIG_PGA_0_512V
#define ADS1115_GAIN_SIXTEEN     ADS1015_REG_CONFIG_PGA_0_256V

typedef struct ADS1115 ADS1115;
struct ADS1115
{
	u16 i2c_address;
	u16 bit_shift;
	u16 gain;
	r32 lsb_size;
	u16 conversion_delay_ms;
	
};

internal void
ADS1115_init(ADS1115 *amp, u16 i2c_address, u16 bit_shift, u16 gain, r32 lsb_size, u16 conversion_delay_ms)
{
	amp->i2c_address = i2c_address;
	amp->bit_shift = bit_shift;
	amp->gain = gain;
	amp->lsb_size = lsb_size;
	amp->conversion_delay_ms = amp->conversion_delay_ms;
}

internal r32
ADS1115_read_adc_diff_01(ADS1115 *adc, OS_I2c *i2c)
{
	//- rsb: write config
	u16 config =
        ADS1015_REG_CONFIG_CQUE_NONE 	 |   // Disable the comparator (default val)
        ADS1015_REG_CONFIG_CLAT_NONLAT    |   // Non-latching (default val)
        ADS1015_REG_CONFIG_CPOL_ACTVLOW   |   // Alert/Rdy active low   (default val)
        ADS1015_REG_CONFIG_CMODE_TRAD 	|   // Traditional comparator (default val)
        ADS1015_REG_CONFIG_DR_1600SPS 	|   // 1600 samples per second (default)
        ADS1015_REG_CONFIG_MODE_SINGLE;   	// Single-shot mode (default)
	config |= adc->gain;
	config |= ADS1015_REG_CONFIG_MUX_DIFF_0_1; // AIN0 = P, AIN1 = N
	config |= ADS1015_REG_CONFIG_OS_SINGLE;
	config = __builtin_bswap16(config);
	os_i2c_write_register8(i2c, adc->i2c_address, ADS1015_REG_POINTER_CONFIG, (u8 *)&config, 2);
	os_delay_ms(adc->conversion_delay_ms);
	
	//- rsb: read result
	i16 lsb_value = 0;
	os_i2c_read_register8(i2c, adc->i2c_address, ADS1015_REG_POINTER_CONVERT, (u8 *)&lsb_value, 2);
	lsb_value = __builtin_bswap16(lsb_value);
	if (adc->bit_shift!= 0) 
	{
		lsb_value >>= adc->bit_shift;
		// Shift 12-bit lsb_values right 4 bits for the ADS1015,
		// making sure we keep the sign bit intact
		if (lsb_value > 0x07FF)
		{
			// negative number - extend the sign to 16th bit
			lsb_value |= 0xF000;
		}
	}
	r32 result = (r32)lsb_value * adc->lsb_size;
	return result;
}


#endif //ADS1115_ADS1015_H
