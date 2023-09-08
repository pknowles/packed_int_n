//
// Copyright (c) 2022-2023, NVIDIA CORPORATION. All rights reserved.
//
// NVIDIA CORPORATION and its licensors retain all intellectual property
// and proprietary rights in and to this software, related documentation
// and any modifications thereto. Any use, reproduction, disclosure or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA CORPORATION is strictly prohibited.
//

// From: https://github.com/NVIDIAGameWorks/Displacement-MicroMap-SDK/blob/main/micromesh_core/include/micromesh/micromesh_utils.h
// See: https://github.com/NVIDIAGameWorks/Displacement-MicroMap-SDK/blob/main/LICENSE.txt
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
// special packed format computations

inline uint32_t packedCountR11UnormPackedAlign32(uint32_t numValues)
{
    return (numValues + 31) / 32;
}
// `bitOffset` starts from data address provided, no range cap
// `bitWidth` must be <= 32
inline void packedBitWrite(void* data, uint32_t bitOffset, uint32_t bitWidth, uint32_t inValue)
{
    uint32_t* outU32 = reinterpret_cast<uint32_t*>(data);

    const uint32_t bitMask = uint32_t((1ull << bitWidth) - 1);
    const uint32_t bitIdx  = bitOffset;
    const uint32_t idx     = bitIdx / 32;
    const uint32_t shift   = bitIdx % 32;

    union
    {
        uint32_t u32s[2];
        uint64_t u64;
    } value;

    union
    {
        uint32_t u32s[2];
        uint64_t u64;
    } mask;

    mask.u64  = uint64_t(bitMask) << shift;
    value.u64 = uint64_t(inValue & bitMask) << shift;

    if(shift + bitWidth <= 32)
    {
        outU32[idx] &= ~mask.u32s[0];
        outU32[idx] |= value.u32s[0];
    }
    else
    {
        outU32[idx] &= ~mask.u32s[0];
        outU32[idx] |= value.u32s[0];
        outU32[idx + 1] &= ~mask.u32s[1];
        outU32[idx + 1] |= value.u32s[1];
    }
}

// `bitOffset` starts from data address provided, no range cap
// `bitWidth` must be <= 32
inline uint32_t packedBitRead(const void* data, uint32_t bitOffset, uint32_t bitWidth)
{
    const uint32_t* outU32 = reinterpret_cast<const uint32_t*>(data);

    const uint32_t bitMask = uint32_t((1ull << bitWidth) - 1);
    const uint32_t bitIdx  = bitOffset;
    const uint32_t idx     = bitIdx / 32;
    const uint32_t shift   = bitIdx % 32;

    if(shift + bitWidth <= 32)
    {
        return (outU32[idx] >> shift) & bitMask;
    }
    else
    {
        union
        {
            uint32_t u32s[2];
            uint64_t u64;
        };
        u32s[0] = outU32[idx];
        u32s[1] = outU32[idx + 1];
        return uint32_t(u64 >> shift) & bitMask;
    }
}

inline void packedWriteR11UnormPackedAlign32(void* data, uint32_t valueIdx, uint32_t inValue)
{
    packedBitWrite(data, valueIdx * 11, 11, inValue);
}

inline uint16_t packedReadR11UnormPackedAlign32(const void* data, uint32_t valueIdx)
{
    return uint16_t(packedBitRead(data, valueIdx * 11, 11));
}
