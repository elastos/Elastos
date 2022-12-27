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
#include "ByteStream.h"

namespace Elastos {
	namespace ElaWallet {
		ByteStream::ByteStream() : _rpos(0) {

		}

		ByteStream::ByteStream(const void *buf, size_t size) : _rpos(0), _buf((const unsigned char *) buf, size) {

		}

		ByteStream::ByteStream(const bytes_t &buf) : _rpos(0), _buf(buf) {

		}

		ByteStream::~ByteStream() {

		}

		void ByteStream::Reset() {
			_rpos = 0;
			_buf.clear();
		}

		void ByteStream::clear() {
			_rpos = 0;
			_buf.clear();
		}

		uint64_t ByteStream::size() const {
			return _buf.size();
		}

		void ByteStream::Skip(size_t bytes) const {
			if (_rpos + bytes <= _buf.size())
				_rpos += bytes;
		}

		const bytes_t &ByteStream::GetBytes() const {
			return _buf;
		}

		bool ByteStream::ReadByte(uint8_t &val) const {
			return ReadBytes(&val, 1);
		}

		bool ByteStream::ReadUint8(uint8_t &val) const {
			return ReadBytes(&val, 1);
		}

		bool ByteStream::ReadUint16(uint16_t &val) const {
			return ReadBytes(&val, sizeof(uint16_t));
		}

		bool ByteStream::ReadUint32(uint32_t &val) const {
			return ReadBytes(&val, sizeof(uint32_t));
		}

		bool ByteStream::ReadUint64(uint64_t &val) const {
			return ReadBytes(&val, sizeof(uint64_t));
		}

		bool ByteStream::ReadBytes(void *buf, size_t len) const {
			if (_rpos + len > _buf.size())
				return false;

			memcpy(buf, &_buf[_rpos], len);
			_rpos += len;

			return true;
		}

		bool ByteStream::ReadBytes(bytes_t &bytes, size_t len) const {
			if (_rpos + len > _buf.size())
				return false;

			bytes.assign(_buf.begin() + _rpos, _buf.begin() + _rpos + len);

			_rpos += len;
			return true;
		}

		bool ByteStream::ReadBytes(uint128 &u) const {
			if (_rpos + u.size() > _buf.size())
				return false;

			memcpy(u.begin(), &_buf[_rpos], u.size());
			_rpos += u.size();
			return true;
		}

		bool ByteStream::ReadBytes(uint160 &u) const {
			if (_rpos + u.size() > _buf.size())
				return false;

			memcpy(u.begin(), &_buf[_rpos], u.size());
			_rpos += u.size();
			return true;
		}

		bool ByteStream::ReadBytes(uint168 &u) const {
			if (_rpos + u.size() > _buf.size())
				return false;

			memcpy(u.begin(), &_buf[_rpos], u.size());
			_rpos += u.size();
			return true;
		}

		bool ByteStream::ReadBytes(uint256 &u) const {
			if (_rpos + u.size() > _buf.size())
				return false;

			memcpy(u.begin(), &_buf[_rpos], u.size());
			_rpos += u.size();
			return true;
		}

		bool ByteStream::ReadVarBytes(bytes_t &bytes) const {
			uint64_t length = 0;
			if (!ReadVarUint(length)) {
				return false;
			}

			return ReadBytes(bytes, length);
		}

		bool ByteStream::ReadVarUint(uint64_t &len) const {
			if (_rpos + 1 > _buf.size())
				return false;

			uint8_t h = _buf[_rpos++];

			switch (h) {
				case VAR_INT16_HEADER:
					if (_rpos + 2 > _buf.size())
						return false;
					len = *(uint16_t *) &_buf[_rpos];
					_rpos += 2;
					break;

				case VAR_INT32_HEADER:
					if (_rpos + 4 > _buf.size())
						return false;
					len = *(uint32_t *) &_buf[_rpos];
					_rpos += 4;
					break;

				case VAR_INT64_HEADER:
					if (_rpos + 8 > _buf.size())
						return false;
					len = *(uint64_t *) &_buf[_rpos];
					_rpos += 8;
					break;

				default:
					len = h;
					break;
			}

			return true;
		}

		bool ByteStream::ReadVarString(std::string &str) const {
			bytes_t bytes;
			if (!ReadVarBytes(bytes)) {
				return false;
			}
			str = std::string((const char *) bytes.data(), bytes.size());

			return true;
		}

		void ByteStream::WriteByte(uint8_t val) {
			_buf.push_back(val);
		}

		void ByteStream::WriteUint8(uint8_t val) {
			_buf.push_back(val);
		}

		void ByteStream::WriteUint16(uint16_t val) {
			WriteBytes(&val, sizeof(uint16_t));
		}

		void ByteStream::WriteUint32(uint32_t val) {
			WriteBytes(&val, sizeof(uint32_t));
		}

		void ByteStream::WriteUint64(uint64_t val) {
			WriteBytes(&val, sizeof(uint64_t));
		}

		void ByteStream::WriteBytes(const void *buf, size_t len) {
			_buf += bytes_t(buf, len);
		}

		void ByteStream::WriteBytes(const bytes_t &bytes) {
			_buf += bytes;
		}

		void ByteStream::WriteBytes(const uint128 &u) {
			_buf += u.bytes();
		}

		void ByteStream::WriteBytes(const uint160 &u) {
			_buf += u.bytes();
		}

		void ByteStream::WriteBytes(const uint168 &u) {
			_buf += u.bytes();
		}

		void ByteStream::WriteBytes(const uint256 &u) {
			_buf += u.bytes();
		}

		void ByteStream::WriteVarBytes(const void *bytes, size_t len) {
			WriteVarUint((uint64_t) len);
			WriteBytes(bytes, len);
		}

		void ByteStream::WriteVarBytes(const bytes_t &bytes) {
			WriteVarUint((uint64_t) bytes.size());
			WriteBytes(bytes);
		}

		size_t ByteStream::WriteVarUint(uint64_t len) {
			size_t count;
			if (len < VAR_INT16_HEADER) {
				_buf.push_back((uint8_t) len);
				count = 1;
			} else if (len <= UINT16_MAX) {
				_buf.push_back(VAR_INT16_HEADER);
				_buf += bytes_t((unsigned char *) &len, 2);
				count = 2;
			} else if (len <= UINT32_MAX) {
				_buf.push_back(VAR_INT32_HEADER);
				_buf += bytes_t((unsigned char *) &len, 4);
				count = 4;
			} else {
				_buf.push_back(VAR_INT64_HEADER);
				_buf += bytes_t((unsigned char *) &len, 8);
				count = 8;
			}
			return count;
		}

		void ByteStream::WriteVarString(const std::string &str) {
			WriteVarBytes(str.c_str(), str.length());
		}
	}
}