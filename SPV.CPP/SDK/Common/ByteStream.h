// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BYTESTREAM_H__
#define __ELASTOS_SDK_BYTESTREAM_H__

#include "CMemBlock.h"
#include <Core/BRAddress.h>

#include <iostream>
#include <stdint.h>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class ByteStream {
		public:

			ByteStream();

			ByteStream(uint64_t size);

			ByteStream(const void *buf, size_t size, bool autorelease = true);

			ByteStream(const CMBlock &buf);

			~ByteStream();

		public:
			void Reset();

			void SetPosition(uint64_t position);

			uint64_t Position();

			uint64_t Length();

			void Drop(size_t bytes);

			CMBlock GetBuffer();

		public:
			bool ReadByte(uint8_t &val);

			bool ReadUint8(uint8_t &val);

			void WriteByte(uint8_t val);

			void WriteUint8(uint8_t val);

			bool ReadUint16(uint16_t &val);

			void WriteUint16(uint16_t val);

			bool ReadUint32(uint32_t &val);

			void WriteUint32(uint32_t val);

			bool ReadUint64(uint64_t &val);

			void WriteUint64(uint64_t val);

			bool ReadBytes(void *buf, size_t len);

			void WriteBytes(const void *buf, size_t len);

			void WriteBytes(const CMBlock &buf);

			bool ReadVarBytes(CMBlock &bytes);

			void WriteVarBytes(const void *bytes, size_t len);

			void WriteVarBytes(const CMBlock &bytes);

			template <class T>
			bool readVarUint(T &value) {
				size_t len = 0;
				uint64_t ui;
				ui = BRVarInt(&_buf[_pos], 9, &len);
				value = ui;
				return ReadBytes(nullptr, len);
			}

			template <class T>
			void writeVarUint(const T &value) {
				size_t len = BRVarIntSet(nullptr, 0, value);

				EnsureCapacity(Position() + len);
				BRVarIntSet(&_buf[Position()], len, value);

				IncreasePosition(len);
				_count = Position();
			}

			bool ReadVarString(std::string &str);

			void WriteVarString(const std::string &str);

		private:
			void IncreasePosition(size_t len);

			void EnsureCapacity(uint64_t newsize);

			bool CheckSize(uint64_t readSize);

		private:
			uint64_t _pos, _count;
			uint64_t _size;
			uint8_t *_buf;
			bool _autorelease;
		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
