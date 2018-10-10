#include "CMemBlock.h"
#include "ByteStream.h"
#include "BRAddress.h"

namespace Elastos {
	namespace ElaWallet {


		ByteStream::ByteStream(bool isBe)
				: _pos(0), _count(0), _size(0), _buf(nullptr), _autorelease(true) ,_isBe(isBe) {
		}

		ByteStream::ByteStream(uint64_t size, bool isBe)
				: _pos(0), _count(0), _size(size), _buf(new uint8_t[size]), _autorelease(true) ,_isBe(isBe) {
			memset(_buf, 0, sizeof(uint8_t) * size);
		}

		ByteStream::ByteStream(uint8_t *buf, uint64_t size, bool autorelease, bool isBe)
				: _pos(0), _count(size), _size(size), _buf(buf), _autorelease(autorelease) ,_isBe(isBe) {}

		ByteStream::~ByteStream() {
			if (_autorelease) {
				delete[]_buf;
				_buf = nullptr;
			}

		}

		void ByteStream::ensureCapacity(uint64_t newsize) {
			if ((int64_t)(newsize - _size) > 0) {
				uint64_t oldCapacity = _size;
				uint64_t newCapacity = oldCapacity << 1;
				int64_t diff = newCapacity - newsize;
				if (diff < 0)
					newCapacity = newsize;
				if (newCapacity <= 0) {
					if (newsize <= 0) // overflow
						return;
					newCapacity = UINT64_MAX;
				}
				uint8_t *newBuf = new uint8_t[newCapacity];
				memset(newBuf, 0, newCapacity);
				memcpy(newBuf, _buf, oldCapacity);
				delete[] _buf;
				_buf = newBuf;
				_size = newCapacity;
			}
		}

		bool ByteStream::checkSize(uint64_t readSize) {
			if (_pos + readSize > _count)
				return false;
			return true;
		}

		void ByteStream::setPosition(uint64_t position) {
			_pos = position;
		}

		uint64_t ByteStream::position() {
			return _pos;
		}

		uint64_t ByteStream::length() {
			return _count;
		}

		uint64_t ByteStream::availableSize() {
			uint64_t ret = _size - _count;

			return ((int64_t)ret) < 0 ? 0 : ret;
		}

		CMBlock ByteStream::getBuffer() {
			if (_count <= 0) {
				return CMBlock();
			}

			CMBlock buff((size_t)_count);
			memcpy(buff, _buf, _count);
			return buff;
		}

		void ByteStream::drop(size_t bytes) {
			if (checkSize(bytes))
				_pos += bytes;
		}

		void ByteStream::reset() {
			this->setPosition(0);
			this->_size = 0;
			if (this->_buf != nullptr) {
				delete[] this->_buf;
				this->_buf = nullptr;
			}
			this->_count = 0;
		}

		void ByteStream::increasePosition(size_t len) {
			_pos += len;
		}

		bool ByteStream::readByte(uint8_t &val) {
			return readBytes(&val, 1);
		}

		bool ByteStream::readUint8(uint8_t &val) {
			return readBytes(&val, 1);
		}

		void ByteStream::writeUint8(uint8_t val) {
			writeBytes(&val, 1);
		}

		void ByteStream::writeByte(uint8_t val) {
			writeBytes(&val, 1);
		}

		bool ByteStream::readUint16(uint16_t &val, ByteOrder byteOrder) {
			return readBytes(&val, sizeof(uint16_t), byteOrder);
		}

		void ByteStream::writeUint16(uint16_t val, ByteOrder byteOrder) {
			writeBytes(&val, sizeof(uint16_t), byteOrder);
		}

		bool ByteStream::readUint32(uint32_t &val, ByteOrder byteOrder) {
			return readBytes(&val, sizeof(uint32_t), byteOrder);
		}

		void ByteStream::writeUint32(uint32_t val, ByteOrder byteOrder) {
			writeBytes(&val, sizeof(uint32_t), byteOrder);
		}

		bool ByteStream::readUint64(uint64_t &val, ByteOrder byteOrder) {
			return readBytes(&val, sizeof(uint64_t), byteOrder);
		}

		void ByteStream::writeUint64(uint64_t val, ByteOrder byteOrder) {
			writeBytes(&val, sizeof(uint64_t), byteOrder);
		}

		bool ByteStream::readBytes(void *buf, size_t len, ByteOrder byteOrder) {
			if (!checkSize(len))
				return false;

			size_t pos = position();

			if (buf != nullptr) {
				if (byteOrder == LittleEndian) {
					memcpy(buf, &_buf[pos], len);
				} else {
					for (size_t i = 0; i < len; ++i) {
						((uint8_t *)buf)[i] = _buf[len - 1 - i + pos];
					}
				}
			}

			increasePosition(len);

			return true;
		}

		void ByteStream::writeBytes(const void *buf, size_t len, ByteOrder byteOrder) {
			ensureCapacity(position() + len);

			size_t pos = position();

			if (byteOrder == LittleEndian) {
				memcpy(&_buf[pos], buf, len);
			} else {
				for (size_t i = 0; i < len; ++i) {
					_buf[pos + i] = ((uint8_t *)buf)[len - i - 1];
				}
			}

			increasePosition(len);
			_count = position();
		}

		void ByteStream::writeBytes(const CMBlock &buf, ByteOrder byteOrder) {
			size_t len = buf.GetSize();

			ensureCapacity(position() + len);

			size_t pos = position();

			if (byteOrder == LittleEndian) {
				memcpy(&_buf[pos], buf, len);
			} else {
				for (size_t i = 0; i < len; ++i) {
					_buf[pos + i] = ((uint8_t *)buf)[len - i - 1];
				}
			}
		}

		bool ByteStream::readVarBytes(CMBlock &bytes) {
			uint64_t length = 0;
			if (!readVarUint(length)) {
				return false;
			}

			bytes.Resize((size_t)length);
			return readBytes(bytes, bytes.GetSize());
		}

		void ByteStream::writeVarBytes(const void *bytes, size_t len) {
			writeVarUint((uint64_t)len);
			writeBytes(bytes, len);
		}

		void ByteStream::writeVarBytes(const CMBlock &bytes) {
			writeVarUint((uint64_t)bytes.GetSize());
			writeBytes(bytes, bytes.GetSize());
		}

		bool ByteStream::readVarUint(uint64_t &value) {
			size_t len = 0;
			value = BRVarInt(&_buf[_pos], 9, &len);
			return readBytes(nullptr, len);
		}

		void ByteStream::writeVarUint(uint64_t value) {
			size_t len = BRVarIntSet(nullptr, 0, value);

			ensureCapacity(position() + len);
			BRVarIntSet(&_buf[position()], len, value);

			increasePosition(len);
			_count = position();
		}

		bool ByteStream::readVarString(char *str, size_t strSize) {
			CMBlock bytes;
			if (!readVarBytes(bytes)) {
				return false;
			}
			size_t len = bytes.GetSize() > strSize - 1 ? strSize - 1 : bytes.GetSize();
			strncpy(str, (const char *)(const void *)bytes, len);
			str[len] = '\0';

			return true;
		}

		bool ByteStream::readVarString(std::string &str) {
			CMBlock bytes;
			if (!readVarBytes(bytes)) {
				return false;
			}
			str = std::string((const char *)(const void *)bytes, (size_t)bytes.GetSize());

			return true;
		}

		void ByteStream::writeVarString(const char *str) {
			writeVarBytes(str, strlen(str));
		}

		void ByteStream::writeVarString(const std::string &str) {
			writeVarBytes(str.c_str(), str.length());
		}

	}
}