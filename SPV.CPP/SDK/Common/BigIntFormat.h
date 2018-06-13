// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <assert.h>

#include "CMemBlock.h"

#ifndef __ELASTOS_SDK_BIGINT_H__
#define __ELASTOS_SDK_BIGINT_H__

namespace Elastos {
	namespace SDK {
		static inline char _toC(uint8_t u) {
			return (u & 0x0f) + ((u & 0x0f) <= 9 ? '0' : 'A' - 0x0a);
		}

		static inline uint8_t _toU(char c) {
			return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - ('a' - 0x0a) : \
             c >= 'A' && c <= 'F' ? c - ('A' - 0x0a) : -1;
		}

		static inline CMemBlock<uint8_t> _toDec(CMemBlock<uint8_t> bigHex) {
			CMemBlock<uint8_t> ret;
			for (size_t i = 0; i < bigHex.GetSize(); i++) {
				uint8_t u = bigHex[i];
				uint8_t alen = 0;
				uint8_t tmp[3] = {0};
				for (size_t j = 0; j < 3; j++) {
					tmp[j] = u % 0x0a;
					alen++;
					u /= 0x0a;
					if (0 == u)
						break;
				}
				size_t off = ret.GetSize();
				ret.Resize(off + 3);
				ret[off++] = tmp[2];
				ret[off++] = tmp[1];
				ret[off++] = tmp[0];
			}
			return ret;
		}

		static inline CMemBlock<char> Hex2Str(CMemBlock<uint8_t> bigNum) {
			CMemBlock<char> ret;
			if (0 < bigNum.GetSize()) {
				ret.Resize(bigNum.GetSize() * 2 + 1);
				ret.Zero();
			}
			for (size_t i = 0; i < bigNum.GetSize(); i++) {
				ret[2 * i] = _toC(bigNum[i] >> 4);
				ret[2 * i + 1] = _toC(bigNum[i]);
			}
			return ret;
		}

		static inline CMemBlock<uint8_t> Str2Hex(CMemBlock<char> bigStr) {
			CMemBlock<uint8_t> ret;
			if (0 < bigStr.GetSize()) {
				assert(1 == bigStr.GetSize() % 2);
				ret.Resize(bigStr.GetSize() / 2);
				for (size_t i = 0; i < ret.GetSize(); i++) {
					ret[i] = (_toU(bigStr[2 * i]) << 4) + _toU(bigStr[2 * i + 1]);
				}
			}
			return ret;
		}

		static inline CMemBlock<char> Dec2Str(CMemBlock<uint8_t> bigHex) {
			CMemBlock<uint8_t> tmp = _toDec(bigHex);
			CMemBlock<char> ret;
			if (0 < tmp.GetSize()) {
				ret.Resize(tmp.GetSize() + 1);
				ret.Zero();
			}
			for (size_t i = 0; i < tmp.GetSize(); i++) {
				char c = _toC(tmp[i]);
				ret[i] = c;
			}
			return ret;
		}

		static inline CMemBlock<uint8_t> Str2Dec(CMemBlock<char> bigDec) {
			CMemBlock<uint8_t> ret;
			assert(1 == bigDec.GetSize() % 3);
			ret.Resize(bigDec.GetSize() / 3);
			ret.Zero();
			for (size_t i = 0; i < ret.GetSize(); i++) {
				ret[i] *= 0x0a;
				ret[i] += _toU(bigDec[3 * i]);
				ret[i] *= 0x0a;
				ret[i] += _toU(bigDec[3 * i + 1]);
				ret[i] *= 0x0a;
				ret[i] += _toU(bigDec[3 * i + 2]);
			}
			return ret;
		}
	}
}

#endif //__ELASTOS_SDK_BIGINT_H_
