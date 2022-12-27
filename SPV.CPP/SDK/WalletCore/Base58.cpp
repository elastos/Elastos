// Created by Aaron Voisine on 9/15/15.
// Copyright (c) 2015 breadwallet LLC
// Copyright (c) 2017-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Base58.h"
#include <Common/uchar_vector.h>
#include <Common/hash.h>
#include <Common/BigInt.h>

namespace Elastos {
	namespace ElaWallet {

#define BITCOIN_BASE58_CHARS "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
#define RIPPLE_BASE58_CHARS  "rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz"

#define DEFAULT_BASE58_CHARS BITCOIN_BASE58_CHARS

		unsigned int Base58::countLeading0s(const bytes_t& data) {
			unsigned int i = 0;
			for (; (i < data.size()) && (data[i] == 0); i++);
			return i;
		}

		unsigned int Base58::countLeading0s(const std::string& numeral, char zeroSymbol) {
			unsigned int i = 0;
			for (; (i < numeral.size()) && (numeral[i] == zeroSymbol); i++);
			return i;
		}

		std::string Base58::CheckEncode(const bytes_t &payload, uint8_t version) {
			const char *pchars = DEFAULT_BASE58_CHARS;
			uchar_vector data;
			data.push_back(version);                                        // prepend version byte
			data += payload;
			uchar_vector checksum = sha256_2(data);
			checksum.assign(checksum.begin(), checksum.begin() + 4);        // compute checksum
			data += checksum;                                               // append checksum
			BigInt bn(data);
			std::string base58check = bn.getInBase(58, pchars);             // convert to base58
			std::string leading0s(countLeading0s(data), pchars[0]);         // prepend leading 0's (1 in base58)
			return leading0s + base58check;
		}

		std::string Base58::CheckEncode(const bytes_t &payload, const bytes_t &version) {
			const char *pchars = DEFAULT_BASE58_CHARS;
			uchar_vector data;
			data += version;                                            // prepend version byte
			data += payload;
			uchar_vector checksum = sha256_2(data);
			checksum.assign(checksum.begin(), checksum.begin() + 4);        // compute checksum
			data += checksum;                                               // append checksum
#if 0
			BigInt bn(data);
			std::string base58check = bn.getInBase(58, pchars);             // convert to base58
			std::string leading0s(countLeading0s(data), pchars[0]);         // prepend leading 0's (1 in base58)
			return leading0s + base58check;
#else
			return Encode(data);
#endif
		}

		bool Base58::CheckDecode(const std::string &base58check, bytes_t &payload, unsigned int &version) {
			const char *pchars = DEFAULT_BASE58_CHARS;
			BigInt bn(base58check, 58, pchars);                                // convert from base58
			uchar_vector bytes = bn.getBytes();
			if (bytes.size() < 4) return false;                                     // not enough bytes
			uchar_vector checksum = uchar_vector(bytes.end() - 4, bytes.end());
			bytes.assign(bytes.begin(), bytes.end() - 4);                           // split string into payload part and checksum part
			uchar_vector leading0s(countLeading0s(base58check, pchars[0]), 0); // prepend leading 0's
			bytes = leading0s + bytes;
			uchar_vector hashBytes = sha256_2(bytes);
			hashBytes.assign(hashBytes.begin(), hashBytes.begin() + 4);
			if (hashBytes != checksum) return false;                                // verify checksum
			version = bytes[0];
			payload.assign(bytes.begin() + 1, bytes.end());
			return true;
		}

		bool Base58::CheckDecode(const std::string &base58check, bytes_t &payload) {
			const char *pchars = DEFAULT_BASE58_CHARS;
			BigInt bn(base58check, 58, pchars);                                // convert from base58
			uchar_vector bytes = bn.getBytes();
			if (bytes.size() < 4) return false;                                     // not enough bytes
			uchar_vector checksum = uchar_vector(bytes.end() - 4, bytes.end());
			bytes.assign(bytes.begin(), bytes.end() - 4);                           // split string into payload part and checksum part
			uchar_vector leading0s(countLeading0s(base58check, pchars[0]), 0); // prepend leading 0's
			bytes = leading0s + bytes;
			uchar_vector hashBytes = sha256_2(bytes);
			hashBytes.assign(hashBytes.begin(), hashBytes.begin() + 4);
			if (hashBytes != checksum) return false;                                // verify checksum
			payload.assign(bytes.begin(), bytes.end());
			return true;
		}

		std::string Base58::Encode(const bytes_t &payload) {
			const char *pchars = DEFAULT_BASE58_CHARS;
#if 0
			BigInt bn(payload);
			std::string base58 = bn.getInBase(58, pchars);
			std::string leading0s(countLeading0s(payload), pchars[0]);
			return leading0s + base58;
#else
			size_t i, j, len, zcount = 0;
			size_t dataLen = payload.size();

			while (zcount < dataLen && payload[zcount] == 0) zcount++; // count leading zeroes

			uint8_t buf[(dataLen - zcount) * 138 / 100 + 1]; // log(256)/log(58), rounded up

			memset(buf, 0, sizeof(buf));

			for (i = zcount; i < dataLen; i++) {
				uint32_t carry = payload[i];

				for (j = sizeof(buf); j > 0; j--) {
					carry += (uint32_t)buf[j - 1] << 8;
					buf[j - 1] = carry % 58;
					carry /= 58;
				}
			}

			i = 0;
			while (i < sizeof(buf) && buf[i] == 0) i++; // skip leading zeroes
			len = (zcount + sizeof(buf) - i) + 1;

			std::string str;
			while (zcount-- > 0) str.push_back(pchars[0]);
			while (i < sizeof(buf)) str.push_back(pchars[buf[i++]]);

			return str;
#endif
		}

		bytes_t Base58::Decode(const std::string &base58) {
			const char *pchars = DEFAULT_BASE58_CHARS;
			BigInt bn(base58, 58, pchars);
			return bn.getBytes();
		}

		bool Base58::Valid(const std::string &base58check) {
			const char *pchars = DEFAULT_BASE58_CHARS;
			BigInt bn(base58check, 58, pchars);                                // convert from base58
			uchar_vector bytes = bn.getBytes();
			uchar_vector checksum = uchar_vector(bytes.end() - 4, bytes.end());
			bytes.assign(bytes.begin(), bytes.end() - 4);                           // split string into payload part and checksum part
			uchar_vector leading0s(countLeading0s(base58check, pchars[0]), 0); // prepend leading 0's
			bytes = leading0s + bytes;
			uchar_vector hashBytes = sha256_2(bytes);
			hashBytes.assign(hashBytes.begin(), hashBytes.begin() + 4);
			return (hashBytes == checksum);
		}

	}
}