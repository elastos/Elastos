// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ByteStream.h"

#include <SDK/Common/CMemBlock.h>

#include <Core/BRAddress.h>

namespace Elastos {
	namespace ElaWallet {


		ByteStream::ByteStream()
				: _pos(0), _count(0), _size(0), _buf(nullptr), _autorelease(true) {
		}

		ByteStream::ByteStream(uint64_t size)
				: _pos(0), _count(0), _size(size), _buf(new uint8_t[size]), _autorelease(true) {
			memset(_buf, 0, sizeof(uint8_t) * size);
		}

		ByteStream::ByteStream(const void *buf, size_t size, bool autorelease)
				: _pos(0), _count(size), _size(size), _buf((uint8_t *) buf), _autorelease(autorelease) {
		}

		ByteStream::ByteStream(const CMBlock &buf)
				: _pos(0), _count(buf.GetSize()), _size(buf.GetSize()), _buf(buf), _autorelease(false) {
		}

		ByteStream::~ByteStream() {
			if (_autorelease) {
				delete[]_buf;
				_buf = nullptr;
			}
		}

		void ByteStream::EnsureCapacity(uint64_t newsize) {
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

		bool ByteStream::CheckSize(uint64_t readSize) {
			if (_pos + readSize > _count)
				return false;
			return true;
		}

		void ByteStream::SetPosition(uint64_t position) {
			_pos = position;
		}

		uint64_t ByteStream::Position() {
			return _pos;
		}

		uint64_t ByteStream::Length() {
			return _count;
		}

		CMBlock ByteStream::GetBuffer() {
			if (_count <= 0) {
				return CMBlock();
			}

			return CMBlock(_buf, (size_t)_count);
		}

		void ByteStream::Drop(size_t bytes) {
			if (CheckSize(bytes))
				_pos += bytes;
		}

		void ByteStream::Reset() {
			this->SetPosition(0);
			this->_size = 0;
			if (this->_buf != nullptr) {
				delete[] this->_buf;
				this->_buf = nullptr;
			}
			this->_count = 0;
		}

		void ByteStream::IncreasePosition(size_t len) {
			_pos += len;
		}

		bool ByteStream::ReadByte(uint8_t &val) {
			return ReadBytes(&val, 1);
		}

		bool ByteStream::ReadUint8(uint8_t &val) {
			return ReadBytes(&val, 1);
		}

		void ByteStream::WriteUint8(uint8_t val) {
			WriteBytes(&val, 1);
		}

		void ByteStream::WriteByte(uint8_t val) {
			WriteBytes(&val, 1);
		}

		bool ByteStream::ReadUint16(uint16_t &val) {
			return ReadBytes(&val, sizeof(uint16_t));
		}

		void ByteStream::WriteUint16(uint16_t val) {
			WriteBytes(&val, sizeof(uint16_t));
		}

		bool ByteStream::ReadUint32(uint32_t &val) {
			return ReadBytes(&val, sizeof(uint32_t));
		}

		void ByteStream::WriteUint32(uint32_t val) {
			WriteBytes(&val, sizeof(uint32_t));
		}

		bool ByteStream::ReadUint64(uint64_t &val) {
			return ReadBytes(&val, sizeof(uint64_t));
		}

		void ByteStream::WriteUint64(uint64_t val) {
			WriteBytes(&val, sizeof(uint64_t));
		}

		bool ByteStream::ReadBytes(void *buf, size_t len) {
			if (!CheckSize(len))
				return false;

			size_t pos = Position();

			if (buf != nullptr) {
				memcpy(buf, &_buf[pos], len);
			}

			IncreasePosition(len);

			return true;
		}

		void ByteStream::WriteBytes(const void *buf, size_t len) {
			EnsureCapacity(Position() + len);

			size_t pos = Position();

			memcpy(&_buf[pos], buf, len);

			IncreasePosition(len);
			_count = Position();
		}

		void ByteStream::WriteBytes(const CMBlock &buf) {
			WriteBytes(buf, buf.GetSize());
		}

		bool ByteStream::ReadVarBytes(CMBlock &bytes) {
			uint64_t length = 0;
			if (!readVarUint(length)) {
				return false;
			}

			if (!CheckSize(length))
				return false;

			bytes = CMBlock((size_t)length);
			return ReadBytes(bytes, bytes.GetSize());
		}

		void ByteStream::WriteVarBytes(const void *bytes, size_t len) {
			writeVarUint((uint64_t)len);
			WriteBytes(bytes, len);
		}

		void ByteStream::WriteVarBytes(const CMBlock &bytes) {
			writeVarUint((uint64_t)bytes.GetSize());
			WriteBytes(bytes, bytes.GetSize());
		}

		bool ByteStream::ReadVarString(std::string &str) {
			CMBlock bytes;
			if (!ReadVarBytes(bytes)) {
				return false;
			}
			str = std::string((const char *)bytes, (size_t)bytes.GetSize());

			return true;
		}

		void ByteStream::WriteVarString(const std::string &str) {
			WriteVarBytes(str.c_str(), str.length());
		}

	}
}
