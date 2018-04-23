#include "ByteStream.h"
#include <stdio.h>
#include <cstring>

namespace Elastos {
	namespace SDK {


		ByteStream::ByteStream()
				: _count(0), _size(0), _buf(NULL), _autorelease(true) {
		}

		ByteStream::ByteStream(int size)
				: _count(0), _size(size), _buf(new uint8_t[size]), _autorelease(true) {
			memset(_buf, 0, sizeof(uint8_t) * size);
		}

		ByteStream::ByteStream(uint8_t *buf, int size, bool autorelease)
				: _count(0), _size(size), _buf(buf), _autorelease(autorelease) {}

		ByteStream::~ByteStream() {
			if (_autorelease) {
				delete[]_buf;
				_buf = NULL;
			}

		}

		void ByteStream::ensureCapacity(int newsize) {
			if (newsize - _size > 0) {
				int oldCapacity = _size;
				int newCapacity = oldCapacity << 1;
				if (newCapacity - newsize < 0)
					newCapacity = newsize;
				if (newCapacity < 0) {
					if (newsize < 0) // overflow
						return;
					newCapacity = INT32_MAX;
				}
				uint8_t *newBuf = new uint8_t[newCapacity];
				memset(newBuf, 0, newCapacity);
				memcpy(newBuf, _buf, oldCapacity);
				delete[] _buf;
				_buf = newBuf;
				_size = newCapacity;
			}
		}

		bool ByteStream::checkSize(int readSize) {
			if (_count + readSize > _size)
				return false;
			return true;
		}

		void ByteStream::setPosition(int position) {
			_count = position;
		}

		int ByteStream::position() {
			return _count;
		}

		int ByteStream::length() {
			return _size;
		}

		void ByteStream::put(uint8_t byte) {
			ensureCapacity(position() + sizeof(uint8_t));
			_buf[_count++] = byte;
		}

		uint8_t ByteStream::get() {
			if (!checkSize(sizeof(uint8_t)))
				return 0;
			return _buf[_count++];
		}

		uint8_t ByteStream::getUByte() {
			return get();
		}

		int8_t ByteStream::getByte() {
			if (!checkSize(sizeof(uint8_t)))
				return 0;
			return _buf[_count++];
		}

		void ByteStream::putShort(int16_t v) {
			ensureCapacity(position() + sizeof(int16_t));
			_buf[_count++] = (v >> 8) & 0xff;
			_buf[_count++] = v & 0xff;
		}

		void ByteStream::putUint16(uint16_t v) {
			ensureCapacity(position() + sizeof(uint16_t));
			_buf[_count++] = (v >> 8) & 0xff;
			_buf[_count++] = v & 0xff;
		}

		int16_t ByteStream::getShort() {
			if (!checkSize(sizeof(int16_t)))
				return 0;
			int16_t r1 = (int16_t) _buf[_count++] << 8;
			int16_t r2 = (int16_t) _buf[_count++];
			int16_t ret = r1 | r2;
			return ret;
		}

		uint16_t ByteStream::getUint16() {
			if (!checkSize(sizeof(uint16_t)))
				return 0;
			uint16_t r1 = (uint16_t) _buf[_count++] << 8;
			uint16_t r2 = (uint16_t) _buf[_count++];
			uint16_t ret = r1 | r2;
			return ret;
		}

		void ByteStream::putInt(int32_t v) {
			ensureCapacity(position() + sizeof(int32_t));
			_buf[_count++] = (v >> 24) & 0xff;
			_buf[_count++] = (v >> 16) & 0xff;
			_buf[_count++] = (v >> 8) & 0xff;
			_buf[_count++] = v & 0xff;
		}

		void ByteStream::putUint32(uint32_t v) {
			ensureCapacity(position() + sizeof(uint32_t));
			_buf[_count++] = (v >> 24) & 0xff;
			_buf[_count++] = (v >> 16) & 0xff;
			_buf[_count++] = (v >> 8) & 0xff;
			_buf[_count++] = v & 0xff;
		}

		int32_t ByteStream::getInt() {
			if (!checkSize(sizeof(int32_t)))
				return 0;
			int32_t ret = (int32_t) _buf[_count++] << 24;
			ret |= (int32_t) _buf[_count++] << 16;
			ret |= (int32_t) _buf[_count++] << 8;
			ret |= (int32_t) _buf[_count++];
			return ret;
		}

		uint32_t ByteStream::getUint32() {
			if (!checkSize(sizeof(uint32_t)))
				return 0;
			uint32_t ret = (uint32_t) _buf[_count++] << 24;
			ret |= (int32_t) _buf[_count++] << 16;
			ret |= (int32_t) _buf[_count++] << 8;
			ret |= (int32_t) _buf[_count++];
			return ret;
		}

		void ByteStream::getInts(int32_t *buf, int32_t len) {
			if (!checkSize(sizeof(int32_t) * len))
				return;
			for (int32_t i = 0; i < len; i++) {
				buf[i] = (int32_t) _buf[_count++] << 24;
				buf[i] |= (int32_t) _buf[_count++] << 16;
				buf[i] |= (int32_t) _buf[_count++] << 8;
				buf[i] |= (int32_t) _buf[_count++];
			}
		}

		void ByteStream::putLong(int64_t v) {
			ensureCapacity(position() + sizeof(int64_t));
			_buf[_count++] = (v >> 56) & 0xff;
			_buf[_count++] = (v >> 48) & 0xff;
			_buf[_count++] = (v >> 40) & 0xff;
			_buf[_count++] = (v >> 32) & 0xff;
			_buf[_count++] = (v >> 24) & 0xff;
			_buf[_count++] = (v >> 16) & 0xff;
			_buf[_count++] = (v >> 8) & 0xff;
			_buf[_count++] = v & 0xff;
		}

		void ByteStream::putUint64(uint64_t v) {
			ensureCapacity(position() + sizeof(uint64_t));
			_buf[_count++] = (v >> 56) & 0xff;
			_buf[_count++] = (v >> 48) & 0xff;
			_buf[_count++] = (v >> 40) & 0xff;
			_buf[_count++] = (v >> 32) & 0xff;
			_buf[_count++] = (v >> 24) & 0xff;
			_buf[_count++] = (v >> 16) & 0xff;
			_buf[_count++] = (v >> 8) & 0xff;
			_buf[_count++] = v & 0xff;
		}

		int64_t ByteStream::getLong() {

			if (!checkSize(sizeof(int64_t)))
				return 0;
			uint64_t ret = 0;
			ret = (int64_t) _buf[_count++] << 56;
			ret |= (int64_t) _buf[_count++] << 48;
			ret |= (int64_t) _buf[_count++] << 40;
			ret |= (int64_t) _buf[_count++] << 32;
			ret |= (int64_t) _buf[_count++] << 24;
			ret |= (int64_t) _buf[_count++] << 16;
			ret |= (int64_t) _buf[_count++] << 8;
			ret |= (int64_t) _buf[_count++];
			return ret;
		}

		uint64_t ByteStream::getUint64() {
			if (!checkSize(sizeof(uint64_t)))
				return 0;
			uint64_t ret = 0;
			ret = (int64_t) _buf[_count++] << 56;
			ret |= (int64_t) _buf[_count++] << 48;
			ret |= (int64_t) _buf[_count++] << 40;
			ret |= (int64_t) _buf[_count++] << 32;
			ret |= (int64_t) _buf[_count++] << 24;
			ret |= (int64_t) _buf[_count++] << 16;
			ret |= (int64_t) _buf[_count++] << 8;
			ret |= (int64_t) _buf[_count++];
			return ret;
		}

		void ByteStream::putBytes(const uint8_t *byte, int len) {
			ensureCapacity(position() + sizeof(uint8_t) * len);
			memcpy(&_buf[position()], byte, len);
			_count += len;
		}

		void ByteStream::getBytes(uint8_t *buf, int len) {
			if (!checkSize(sizeof(uint8_t) * len))
				return;
			memcpy(buf, &_buf[_count], len);
			_count += len;
		}

		void ByteStream::putUTF8(const char *str) {
			int len = 0;
			int idx = 0;
			while (str[idx] != '\0') {
				len++;
				idx++;
			}
			putShort(len);
			putBytes((uint8_t *) str, len);
		}

		char *ByteStream::getUTF8(int32_t &len) {
			short utfLen = getShort();
			char *utfBuffer = new char[utfLen + 1];
			if (!checkSize(utfLen)) {
				utfBuffer[0] = '\0';
			} else {
				getBytes((uint8_t *) utfBuffer, utfLen);
				utfBuffer[utfLen] = '\0';
			}

			len = utfLen + 2;

			return utfBuffer;
		}

		size_t ByteStream::availableSize() {
			size_t ret = _size - _count;

			return ret;
		}

		char *ByteStream::getUTF8() {
			int len = 0;
			return getUTF8(len);
		}

		uint8_t *ByteStream::getBuf() {
			uint8_t *ret = new uint8_t[_size];
			memcpy(ret, _buf, _size);
			return ret;
		}

		void ByteStream::skip(int bytes) {
			if (checkSize(bytes))
				_count += bytes;
		}

		void ByteStream::reSet() {
			this->setPosition(0);
			this->_size = 0;
			this->_buf = nullptr;
		}

	}
}