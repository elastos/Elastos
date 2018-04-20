// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "BRInt.h"

int UInt128Eq(const UInt128* a, const UInt128* b)
{
	return (a->u64[0] == b->u64[0] && a->u64[1] == b->u64[1]);
}

int UInt160Eq(const UInt160* a, const UInt160* b)
{
	return (a->u32[0] == b->u32[0] && a->u32[1] == b->u32[1] && a->u32[2] == b->u32[2] && a->u32[3] == b->u32[3] &&
	        a->u32[4] == b->u32[4]);
}

int UInt168Eq(const UInt168* a, const UInt168* b)
{
	for(short i = 0; i < 21; i++)
	{
		if(a->u8[i] != b->u8[i])
		{
			return 0;
		}
	}
	return 1;
}

int UInt256Eq(const UInt256* a, const UInt256* b)
{
	return (a->u64[0] == b->u64[0] && a->u64[1] == b->u64[1] && a->u64[2] == b->u64[2] && a->u64[3] == b->u64[3]);
}

int UInt512Eq(const UInt512* a, const UInt512* b)
{
	return (a->u64[0] == b->u64[0] && a->u64[1] == b->u64[1] && a->u64[2] == b->u64[2] && a->u64[3] == b->u64[3] &&
	        a->u64[4] == b->u64[4] && a->u64[5] == b->u64[5] && a->u64[6] == b->u64[6] && a->u64[7] == b->u64[7]);
}

int UInt128IsZero(const UInt128* u)
{
	return ((u->u64[0] | u->u64[1]) == 0);
}

int UInt160IsZero(const UInt160* u)
{
	return ((u->u32[0] | u->u32[1] | u->u32[2] | u->u32[3] | u->u32[4]) == 0);
}

int UInt168IsZero(const UInt168* u)
{
	int value = 0;
	for(short i = 0; i < 21; i++)
	{
		value = value | u->u8[i];
		if(value != 0)
		{
			break;
		}
	}
	return value == 0;
}

int UInt256IsZero(const UInt256* u)
{
	return ((u->u64[0] | u->u64[1] | u->u64[2] | u->u64[3]) == 0);
}

int UInt512IsZero(const UInt512* u)
{
	return ((u->u64[0] | u->u64[1] | u->u64[2] | u->u64[3] | u->u64[4] | u->u64[5] | u->u64[6] | u->u64[7]) == 0);
}

UInt256 UInt256Reverse(const UInt256* u)
{
	return ((UInt256) { .u8 = { u->u8[31], u->u8[30], u->u8[29], u->u8[28], u->u8[27], u->u8[26], u->u8[25], u->u8[24],
	                            u->u8[23], u->u8[22], u->u8[21], u->u8[20], u->u8[19], u->u8[18], u->u8[17], u->u8[16],
	                            u->u8[15], u->u8[14], u->u8[13], u->u8[12], u->u8[11], u->u8[10], u->u8[ 9], u->u8[ 8],
	                            u->u8[ 7], u->u8[ 6], u->u8[5],  u->u8[ 4], u->u8[ 3], u->u8[ 2], u->u8[ 1], u->u8[ 0] }
	                   });
}

void UInt8SetBE(void *b2, uint16_t u)
{
//	todo complete me
}

void UInt8SetLE(void *b2, uint16_t u)
{
//	todo complete me
}

void UInt16SetBE(void *b2, uint16_t u)
{
    *(union _u16 *)b2 = (union _u16) { (u >> 8) & 0xff, u & 0xff };
}

void UInt16SetLE(void *b2, uint16_t u)
{
    *(union _u16 *)b2 = (union _u16) { u & 0xff, (u >> 8) & 0xff };
}

void UInt32SetBE(void *b4, uint32_t u)
{
    *(union _u32 *)b4 =
            (union _u32) { (u >> 24) & 0xff, (u >> 16) & 0xff, (u >> 8) & 0xff, u & 0xff };
}

void UInt32SetLE(void *b4, uint32_t u)
{
    *(union _u32 *)b4 =
            (union _u32) { u & 0xff, (u >> 8) & 0xff, (u >> 16) & 0xff, (u >> 24) & 0xff };
}

void UInt64SetBE(void *b8, uint64_t u)
{
    *(union _u64 *)b8 =
            (union _u64) { (u >> 56) & 0xff, (u >> 48) & 0xff, (u >> 40) & 0xff, (u >> 32) & 0xff,
                           (u >> 24) & 0xff, (u >> 16) & 0xff, (u >> 8) & 0xff, u & 0xff };
}

void UInt64SetLE(void *b8, uint64_t u)
{
    *(union _u64 *)b8 =
            (union _u64) { u & 0xff, (u >> 8) & 0xff, (u >> 16) & 0xff, (u >> 24) & 0xff,
                           (u >> 32) & 0xff, (u >> 40) & 0xff, (u >> 48) & 0xff, (u >> 56) & 0xff };
}

void UInt128Set(void *b16, UInt128 u)
{
    *(union _u128 *)b16 =
            (union _u128) { u.u8[0], u.u8[1], u.u8[2],  u.u8[3],  u.u8[4],  u.u8[5],  u.u8[6],  u.u8[7],
                            u.u8[8], u.u8[9], u.u8[10], u.u8[11], u.u8[12], u.u8[13], u.u8[14], u.u8[15] };
}

void UInt160Set(void *b20, UInt160 u)
{
    *(union _u160 *)b20 =
            (union _u160) { u.u8[0],  u.u8[1],  u.u8[2],  u.u8[3],  u.u8[4],  u.u8[5],  u.u8[6],  u.u8[7],
                            u.u8[8],  u.u8[9],  u.u8[10], u.u8[11], u.u8[12], u.u8[13], u.u8[14], u.u8[15],
                            u.u8[16], u.u8[17], u.u8[18], u.u8[19] };
}

void UInt168Set(void *b21, UInt168 u)
{
    *(union _u168 *)b21 =
            (union _u168) { u.u8[0],  u.u8[1],  u.u8[2],  u.u8[3],  u.u8[4],  u.u8[5],  u.u8[6],  u.u8[7],
                            u.u8[8],  u.u8[9],  u.u8[10], u.u8[11], u.u8[12], u.u8[13], u.u8[14], u.u8[15],
                            u.u8[16], u.u8[17], u.u8[18], u.u8[19], u.u8[20] };
}

void UInt256Set(void *b32, UInt256 u)
{
    *(union _u256 *)b32 =
            (union _u256) { u.u8[0],  u.u8[1],  u.u8[2],  u.u8[3],  u.u8[4],  u.u8[5],  u.u8[6],  u.u8[7],
                            u.u8[8],  u.u8[9],  u.u8[10], u.u8[11], u.u8[12], u.u8[13], u.u8[14], u.u8[15],
                            u.u8[16], u.u8[17], u.u8[18], u.u8[19], u.u8[20], u.u8[21], u.u8[22], u.u8[23],
                            u.u8[24], u.u8[25], u.u8[26], u.u8[27], u.u8[28], u.u8[29], u.u8[30], u.u8[31] };
}

uint8_t UInt8GetBE(const void *b2)
{
	//	todo complete me
	return 0;
}

uint8_t UInt8GetLE(const void *b2)
{
	//	todo complete me
	return 0;
}

uint16_t UInt16GetBE(const void *b2)
{
    return (((uint16_t)((const uint8_t *)b2)[0] << 8) | ((uint16_t)((const uint8_t *)b2)[1]));
}

uint16_t UInt16GetLE(const void *b2)
{
    return (((uint16_t)((const uint8_t *)b2)[1] << 8) | ((uint16_t)((const uint8_t *)b2)[0]));
}

uint32_t UInt32GetBE(const void *b4)
{
    return (((uint32_t)((const uint8_t *)b4)[0] << 24) | ((uint32_t)((const uint8_t *)b4)[1] << 16) |
            ((uint32_t)((const uint8_t *)b4)[2] << 8)  | ((uint32_t)((const uint8_t *)b4)[3]));
}

uint32_t UInt32GetLE(const void *b4)
{
    return (((uint32_t)((const uint8_t *)b4)[3] << 24) | ((uint32_t)((const uint8_t *)b4)[2] << 16) |
            ((uint32_t)((const uint8_t *)b4)[1] << 8)  | ((uint32_t)((const uint8_t *)b4)[0]));
}

uint64_t UInt64GetBE(const void *b8)
{
    return (((uint64_t)((const uint8_t *)b8)[0] << 56) | ((uint64_t)((const uint8_t *)b8)[1] << 48) |
            ((uint64_t)((const uint8_t *)b8)[2] << 40) | ((uint64_t)((const uint8_t *)b8)[3] << 32) |
            ((uint64_t)((const uint8_t *)b8)[4] << 24) | ((uint64_t)((const uint8_t *)b8)[5] << 16) |
            ((uint64_t)((const uint8_t *)b8)[6] << 8)  | ((uint64_t)((const uint8_t *)b8)[7]));
}

uint64_t UInt64GetLE(const void *b8)
{
    return (((uint64_t)((const uint8_t *)b8)[7] << 56) | ((uint64_t)((const uint8_t *)b8)[6] << 48) |
            ((uint64_t)((const uint8_t *)b8)[5] << 40) | ((uint64_t)((const uint8_t *)b8)[4] << 32) |
            ((uint64_t)((const uint8_t *)b8)[3] << 24) | ((uint64_t)((const uint8_t *)b8)[2] << 16) |
            ((uint64_t)((const uint8_t *)b8)[1] << 8)  | ((uint64_t)((const uint8_t *)b8)[0]));
}

void UInt128Get(UInt128* value, const void *b16)
{
	for(uint8_t i = 0; i < 16; i++){
		value->u8[i] = ((const uint8_t *)b16)[i];
	}
}

void UInt160Get(UInt160* value, const void *b20)
{
	for(uint8_t i = 0; i < 20; i++){
		value->u8[i] = ((const uint8_t *)b20)[i];
	}
}

void UInt168Get(UInt168* value, const void *b21)
{
	for(uint8_t i = 0; i < 21; i++){
		value->u8[i] = ((const uint8_t *)b21)[i];
	}
}

void UInt256Get(UInt256* value, const void *b32)
{
	for(uint8_t i = 0; i < 32; i++){
		value->u8[i] = ((const uint8_t *)b32)[i];
	}
}