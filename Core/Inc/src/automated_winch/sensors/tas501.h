
#ifndef TAS501_H
#define TAS501_H

typedef struct TAS501 TAS501;
struct TAS501
{
	r32 excitation_voltage_V;
	r32 max_capacity_kg;
	r32 sensitivity_mV_per_V; // mV/V
	r32 kg_from_mV; // kg/mV
};

internal void
TAS501_init(TAS501 *strain_gauge, r32 excitation_voltage_V, r32 max_capacity_kg, r32 sensitivity_mV_per_V)
{
	strain_gauge->excitation_voltage_V = excitation_voltage_V;
	strain_gauge->max_capacity_kg = max_capacity_kg;
	strain_gauge->sensitivity_mV_per_V = sensitivity_mV_per_V;
	strain_gauge->kg_from_mV = max_capacity_kg / (excitation_voltage_V * sensitivity_mV_per_V);
}

internal r32
TAS501_tension_kg(TAS501 *strain_gauge, r32 adc_mV)
{
	r32 result = adc_mV * strain_gauge->kg_from_mV;
	return result;
}

#endif //TAS501_H
