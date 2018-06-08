#ifndef __ELASTOS_SDK_BYTESTREAM_H__
#define __ELASTOS_SDK_BYTESTREAM_H__

#include <iostream>
#include <stdint.h>
#include <vector>

namespace Elastos {
	namespace SDK {

		class ByteStream {
		public:

			ByteStream(bool isBe = false);

			ByteStream(uint64_t size, bool isBe = false);

			ByteStream(uint8_t *buf, uint64_t size, bool autorelease = true, bool isBe = false);

			~ByteStream();

		public:
			void reSet();

			void setPosition(uint64_t position);

			uint64_t position();

			uint64_t length();

			uint64_t availableSize();

			void skip(int bytes);

			uint8_t *getBuf();

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

		private:
			void ensureCapacity(uint64_t newsize);

			bool checkSize(uint64_t readSize);

		private:
			uint64_t _count;
			uint64_t _size;
			uint8_t *_buf;
			bool _autorelease;
			bool _isBe;
		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
