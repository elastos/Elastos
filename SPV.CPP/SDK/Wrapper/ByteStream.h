#ifndef __ELASTOS_SDK_BYTESTREAM_H__
#define __ELASTOS_SDK_BYTESTREAM_H__

#include <iostream>
#include <stdint.h>
#include <vector>

namespace Elastos {
	namespace SDK {

		class ByteStream {
		public:

			ByteStream();

			ByteStream(int size);

			ByteStream(uint8_t *buf, int size, bool autorelease = true);

			~ByteStream();

		public:
			void reSet();

			void setPosition(int position);

			int position();

			int length();

			size_t availableSize();

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

			void putBytes(const uint8_t *byte, int len);

			void putFloat(float floatValue);

			void putUTF8(const char *str);

			void putUTF16BE(const char *str);

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

			float getFloat();

			double getDouble();

			char *getUTF8();

			char *getUTF8(int32_t &len);

			void getBytes(uint8_t *buf, int32_t len);

			void getInts(int32_t *buf, int32_t len);

		public:

			int32_t _count;
			int32_t _size;
			uint8_t *_buf;
			bool _autorelease;

		private:

			void ensureCapacity(int newsize);

			bool checkSize(int readSize);

			static int32_t floatToIntBits(float value);

			static int64_t doubleToLongBits(double value);

		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
