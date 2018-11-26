// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BYTESTREAM_H__
#define __ELASTOS_SDK_BYTESTREAM_H__

#include "CMemBlock.h"

#include <iostream>
#include <stdint.h>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class ByteStream {
			enum ByteOrder {
				LittleEndian,
				BigEndian
			};
		public:

			ByteStream();

			ByteStream(uint64_t size);

			ByteStream(const void *buf, size_t size, bool autorelease = true);

			ByteStream(const CMBlock &buf);

			~ByteStream();

		public:
			void reset();

			void setPosition(uint64_t position);

			uint64_t position();

			uint64_t length();

			void drop(size_t bytes);

			CMBlock getBuffer();

		public:
			void increasePosition(size_t len);

			bool readByte(uint8_t &val);

			bool readUint8(uint8_t &val);

			void writeByte(uint8_t val);

			void writeUint8(uint8_t val);

			bool readUint16(uint16_t &val, ByteOrder byteOrder = LittleEndian);

			void writeUint16(uint16_t val, ByteOrder byteOrder = LittleEndian);

			bool readUint32(uint32_t &val, ByteOrder byteOrder = LittleEndian);

			void writeUint32(uint32_t val, ByteOrder byteOrder = LittleEndian);

			bool readUint64(uint64_t &val, ByteOrder byteOrder = LittleEndian);

			void writeUint64(uint64_t val, ByteOrder byteOrder = LittleEndian);

			bool readBytes(void *buf, size_t len, ByteOrder byteOrder = LittleEndian);

			void writeBytes(const void *buf, size_t len, ByteOrder byteOrder = LittleEndian);

			void writeBytes(const CMBlock &buf, ByteOrder byteOrder = LittleEndian);

			bool readVarBytes(CMBlock &bytes);

			void writeVarBytes(const void *bytes, size_t len);

			void writeVarBytes(const CMBlock &bytes);

			bool readVarUint(uint64_t &value);

			void writeVarUint(uint64_t value);

			bool readVarString(char *str, size_t strSize);

			bool readVarString(std::string &str);

			void writeVarString(const char *str);

			void writeVarString(const std::string &str);

		private:
			void ensureCapacity(uint64_t newsize);

			bool checkSize(uint64_t readSize);

		private:
			uint64_t _pos, _count;
			uint64_t _size;
			uint8_t *_buf;
			bool _autorelease;
		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
