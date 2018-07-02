#ifndef __ELASTOS_SDK_BYTESTREAM_H__
#define __ELASTOS_SDK_BYTESTREAM_H__

#include <iostream>
#include <stdint.h>
#include <vector>

#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class ByteStream {
			enum ByteOrder {
				LittleEndian,
				BigEndian
			};
		public:

			ByteStream(bool isBe = false);

			ByteStream(uint64_t size, bool isBe = false);

			ByteStream(uint8_t *buf, uint64_t size, bool autorelease = true, bool isBe = false);

			~ByteStream();

		public:
			void reset();

			void setPosition(uint64_t position);

			uint64_t position();

			uint64_t length();

			uint64_t availableSize();

			void skip(int bytes);

			CMBlock getBuffer();

		public:
			void put(uint8_t byte);

			void putShort(int16_t v);

			void putUint16(uint16_t v);

			void putInt(int32_t v);


			void putUint32(uint32_t v);

			void putLong(int64_t v);

			void putUint64(uint64_t v);

			void putBytes(const uint8_t *byte, uint64_t len);

			void putUTF8(const char *str);

			void putVarUint(uint64_t value);

		public:
			uint8_t get();

			int8_t getByte();

			uint8_t getUByte();

			int16_t getShort();

			int32_t getInt();

			uint32_t getUint32();


			uint64_t getUint64();

			uint16_t getUint16();

			int64_t getLong();

			char *getUTF8();

			char *getUTF8(int32_t &len);

			void getBytes(uint8_t *buf, uint64_t len);

			void getInts(int32_t *buf, int32_t len);

			uint64_t getVarUint();

		public:
			void increasePosition(size_t len);
			bool readUint8(uint8_t &val);
			void writeUint8(uint8_t val);
			bool readUint16(uint16_t &val, ByteOrder byteOrder = LittleEndian);
			void writeUint16(uint16_t val, ByteOrder byteOrder = LittleEndian);
			bool readUint32(uint32_t &val, ByteOrder byteOrder = LittleEndian);
			void writeUint32(uint32_t val, ByteOrder byteOrder = LittleEndian);
			bool readUint64(uint64_t &val, ByteOrder byteOrder = LittleEndian);
			void writeUint64(uint64_t val, ByteOrder byteOrder = LittleEndian);
			bool readBytes(void *buf, size_t len, ByteOrder byteOrder = LittleEndian);
			void writeBytes(const void *buf, size_t len, ByteOrder byteOrder = LittleEndian);
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
			bool _isBe;
		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
