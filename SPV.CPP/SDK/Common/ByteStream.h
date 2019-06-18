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

			ByteStream() :
				_rpos(0) {
			}

			ByteStream(const void *buf, size_t size)
				: _rpos(0), _buf((const unsigned char *)buf, size)  {
			}

			explicit ByteStream(const bytes_t &buf)
				: _rpos(0), _buf(buf) {
			}

			~ByteStream() {
			}

			void Reset() {
				_rpos = 0;
				_buf.clear();
			}

			uint64_t size() const {
				return _buf.size();
			}

			void Skip(size_t bytes = 1) const {
				if (_rpos + bytes <= _buf.size())
					_rpos += bytes;
			}

			const bytes_t &GetBytes() const {
				return _buf;
			}

			bool ReadByte(uint8_t &val) const {
				return ReadBytes(&val, 1);
			}

			bool ReadUint8(uint8_t &val) const {
				return ReadBytes(&val, 1);
			}

			bool ReadUint16(uint16_t &val) const {
				return ReadBytes(&val, sizeof(uint16_t));
			}

			bool ReadUint32(uint32_t &val) const {
				return ReadBytes(&val, sizeof(uint32_t));
			}

			bool ReadUint64(uint64_t &val) const {
				return ReadBytes(&val, sizeof(uint64_t));
			}

			bool ReadBytes(void *buf, size_t len) const {
				if (_rpos + len > _buf.size())
					return false;

				memcpy(buf, &_buf[_rpos], len);
				_rpos += len;

				return true;
			}

			bool ReadBytes(bytes_t &bytes, size_t len) const {
				if (_rpos + len > _buf.size())
					return false;

				bytes.assign(_buf.begin() + _rpos, _buf.begin() + _rpos + len);

				_rpos += len;
				return true;
			}

			bool ReadBytes(uint128 &u) const {
				if (_rpos + u.size() > _buf.size())
					return false;

				memcpy(u.begin(), &_buf[_rpos], u.size());
				_rpos += u.size();
				return true;
			}

			bool ReadBytes(uint160 &u) const {
				if (_rpos + u.size() > _buf.size())
					return false;

				memcpy(u.begin(), &_buf[_rpos], u.size());
				_rpos += u.size();
				return true;
			}

			bool ReadBytes(uint168 &u) const {
				if (_rpos + u.size() > _buf.size())
					return false;

				memcpy(u.begin(), &_buf[_rpos], u.size());
				_rpos += u.size();
				return true;
			}

			bool ReadBytes(uint256 &u) const {
				if (_rpos + u.size() > _buf.size())
					return false;

				memcpy(u.begin(), &_buf[_rpos], u.size());
				_rpos += u.size();
				return true;
			}

			bool ReadVarBytes(bytes_t &bytes) const {
				uint64_t length = 0;
				if (!ReadVarUint(length)) {
					return false;
				}

				return ReadBytes(bytes, length);
			}

			bool ReadVarUint(uint64_t &len) const {
				if (_rpos + 1 > _buf.size())
					return false;

				uint8_t h = _buf[_rpos++];

				switch (h) {
					case VAR_INT16_HEADER:
						if (_rpos + 2 > _buf.size())
							return false;
						len = *(uint16_t *)&_buf[_rpos];
						_rpos += 2;
						break;

					case VAR_INT32_HEADER:
						if (_rpos + 4 > _buf.size())
							return false;
						len = *(uint32_t *)&_buf[_rpos];
						_rpos += 4;
						break;

					case VAR_INT64_HEADER:
						if (_rpos + 8 > _buf.size())
							return false;
						len = *(uint64_t *)&_buf[_rpos];
						_rpos += 8;
						break;

					default:
						len = h;
						break;
				}

				return true;
			}

			bool ReadVarString(std::string &str) const {
				bytes_t bytes;
				if (!ReadVarBytes(bytes)) {
					return false;
				}
				str = std::string((const char *)bytes.data(), bytes.size());

				return true;
			}

			void WriteByte(uint8_t val) {
				_buf.push_back(val);
			}

			void WriteUint8(uint8_t val) {
				_buf.push_back(val);
			}

			void WriteUint16(uint16_t val) {
				WriteBytes(&val, sizeof(uint16_t));
			}

			void WriteUint32(uint32_t val) {
				WriteBytes(&val, sizeof(uint32_t));
			}

			void WriteUint64(uint64_t val) {
				WriteBytes(&val, sizeof(uint64_t));
			}

			void WriteBytes(const void *buf, size_t len) {
				_buf += bytes_t(buf, len);
			}

			void WriteBytes(const bytes_t &bytes) {
				_buf += bytes;
			}

			void WriteBytes(const uint128 &u) {
				_buf += u.bytes();
			}

			void WriteBytes(const uint160 &u) {
				_buf += u.bytes();
			}

			void WriteBytes(const uint168 &u) {
				_buf += u.bytes();
			}

			void WriteBytes(const uint256 &u) {
				_buf += u.bytes();
			}

			void WriteVarBytes(const void *bytes, size_t len) {
				WriteVarUint((uint64_t)len);
				WriteBytes(bytes, len);
			}

			void WriteVarBytes(const bytes_t &bytes) {
				WriteVarUint((uint64_t)bytes.size());
				WriteBytes(bytes);
			}

			size_t WriteVarUint(uint64_t len) {
				size_t count;
				if (len < VAR_INT16_HEADER) {
					_buf.push_back((uint8_t)len);
					count = 1;
				} else if (len <= UINT16_MAX) {
					_buf.push_back(VAR_INT16_HEADER);
					_buf += bytes_t((unsigned char *)&len, 2);
					count = 2;
				} else if (len <= UINT32_MAX) {
					_buf.push_back(VAR_INT32_HEADER);
					_buf += bytes_t((unsigned char *)&len, 4);
					count = 4;
				} else {
					_buf.push_back(VAR_INT64_HEADER);
					_buf += bytes_t((unsigned char *)&len, 8);
					count = 8;
				}
				return count;
			}

			void WriteVarString(const std::string &str) {
				WriteVarBytes(str.c_str(), str.length());
			}

		private:
			mutable size_t _rpos;
			bytes_t _buf;
		};

	}
}

#endif //__ELASTOS_SDK_BYTESTREAM_H__
