
#ifndef BASE_MATH_H
#define BASE_MATH_H

internal u32 
sdbm_checksum(u8 *data, u32 size)
{
    u32 checksum = 0;
    while(size--)
	{
        checksum = checksum * 65599 + *data++;
    }
    return checksum;
}

typedef union Rng2F32 Rng2F32;
union Rng2F32
{
	struct
	{
		r32 min;
		r32 max;
	};
	r32 v[2];
};

internal Rng2F32
rng2f32(r32 min, r32 max)
{
	Rng2F32 result = (Rng2F32){.min = Min(min, max), .max = Max(min, max)};
	return result;
}

internal r32
clamp_rng2f32(Rng2F32 range, r32 val)
{
	r32 result = Clamp(range.min, val, range.max);
	return result;
}

#endif //BASE_MATH_H
