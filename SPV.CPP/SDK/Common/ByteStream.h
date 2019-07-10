// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
