/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ELASTOS_SDK_BYTESTREAM_H__
#define __ELASTOS_SDK_BYTESTREAM_H__

#include "typedefs.h"
#include "uint256.h"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class ByteStream {
		public:

#define VAR_INT16_HEADER  0xfd
#define VAR_INT32_HEADER  0xfe
#define VAR_INT64_HEADER  0xff
#define MAX_SCRIPT_LENGTH 0x100 // scripts over this size will not be parsed for an address

			ByteStream();

			ByteStream(const void *buf, size_t size);

			explicit ByteStream(const bytes_t &buf);

			~ByteStream();

			void Reset();

			void clear();

			uint64_t size() const;

			void Skip(size_t bytes = 1) const;

			const bytes_t &GetBytes() const;

			bool ReadByte(uint8_t &val) const;

			bool ReadUint8(uint8_t &val) const;

			bool ReadUint16(uint16_t &val) const;

			bool ReadUint32(uint32_t &val) const;

			bool ReadUint64(uint64_t &val) const;

			bool ReadBytes(void *buf, size_t len) const;

			bool ReadBytes(bytes_t &bytes, size_t len) const;

			bool ReadBytes(uint128 &u) const;

			bool ReadBytes(uint160 &u) const;

			bool ReadBytes(uint168 &u) const;

			bool ReadBytes(uint256 &u) const;

			bool ReadVarBytes(bytes_t &bytes) const;

			bool ReadVarUint(uint64_t &len) const;

			bool ReadVarString(std::string &str) const;

			void WriteByte(uint8_t val);

			void WriteUint8(uint8_t val);

			void WriteUint16(uint16_t val);

			void WriteUint32(uint32_t val);

			void WriteUint64(uint64_t val);

			void WriteBytes(const void *buf, size_t len);

			void WriteBytes(const bytes_t &bytes);

			void WriteBytes(const uint128 &u);

			void WriteBytes(const uint160 &u);

			void WriteBytes(const uint168 &u);

			void WriteBytes(const uint256 &u);

			void WriteVarBytes(const void *bytes, size_t len);

			void WriteVarBytes(const bytes_t &bytes);

			size_t WriteVarUint(uint64_t len);

			void WriteVarString(const std::string &str);

		private:
			mutable size_t _rpos;
			bytes_t _buf;
		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
