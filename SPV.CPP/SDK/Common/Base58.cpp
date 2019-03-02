// Created by Aaron Voisine on 9/15/15.
// Copyright (c) 2015 breadwallet LLC
// Copyright (c) 2017-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SDK/Common/Base58.h"
#include <Core/BRCrypto.h>

namespace Elastos {
	namespace ElaWallet {

		// base58 and base58check encoding: https://en.bitcoin.it/wiki/Base58Check_encoding

		// returns the number of characters written to str including NULL terminator, or total strLen needed if str is NULL
		std::string Base58::Encode(const void *data, size_t dataLen) {
			static const char chars[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
			size_t i, j, len, zcount = 0;
			const uint8_t *pdata = (const uint8_t *)data;

			while (zcount < dataLen && pdata[zcount] == 0) zcount++; // count leading zeroes

			uint8_t buf[(dataLen - zcount)*138/100 + 1]; // log(256)/log(58), rounded up

			memset(buf, 0, sizeof(buf));

			for (i = zcount; data && i < dataLen; i++) {
				uint32_t carry = pdata[i];

				for (j = sizeof(buf); j > 0; j--) {
					carry += (uint32_t)buf[j - 1] << 8;
					buf[j - 1] = carry % 58;
					carry /= 58;
				}

				var_clean(&carry);
			}

			i = 0;
			while (i < sizeof(buf) && buf[i] == 0) i++; // skip leading zeroes
			len = (zcount + sizeof(buf) - i) + 1;
			char str[len], *pstr;
			pstr = str;

			while (zcount-- > 0) *(pstr++) = chars[0];
			while (i < sizeof(buf)) *(pstr++) = chars[buf[i++]];
			*pstr = '\0';

			mem_clean(buf, sizeof(buf));
			return std::string(str);
		}

		// returns the number of bytes written to data, or total dataLen needed if data is NULL
		CMBlock Base58::Decode(const std::string &str) {
			size_t i = 0, j, len, zcount = 0;
			const char *pstr = str.c_str();

			while (*pstr == '1') pstr++, zcount++; // count leading zeroes

			uint8_t buf[str.length()*733/1000 + 1]; // log(58)/log(256), rounded up

			memset(buf, 0, sizeof(buf));

			while (*pstr) {
				uint32_t carry = *(const uint8_t *)(pstr++);

				switch (carry) {
					case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						carry -= '1';
						break;

					case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H':
						carry += 9 - 'A';
						break;

					case 'J': case 'K': case 'L': case 'M': case 'N':
						carry += 17 - 'J';
						break;

					case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y':
					case 'Z':
						carry += 22 - 'P';
						break;

					case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
					case 'k':
						carry += 33 - 'a';
						break;

					case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v':
					case 'w': case 'x': case 'y': case 'z':
						carry += 44 - 'm';
						break;

					default:
						carry = UINT32_MAX;
				}

				if (carry >= 58) break; // invalid base58 digit

				for (j = sizeof(buf); j > 0; j--) {
					carry += (uint32_t)buf[j - 1]*58;
					buf[j - 1] = carry & 0xff;
					carry >>= 8;
				}

				var_clean(&carry);
			}

			while (i < sizeof(buf) && buf[i] == 0) i++; // skip leading zeroes
			len = zcount + sizeof(buf) - i;
			CMBlock data(len);

			if (zcount > 0) memset(data, 0, zcount);
			memcpy(&data[zcount], &buf[i], sizeof(buf) - i);

			mem_clean(buf, sizeof(buf));
			return data;
		}

		// returns the number of characters written to str including NULL terminator, or total strLen needed if str is NULL
		std::string Base58::CheckEncode(const void *data, size_t dataLen) {
			size_t bufLen = dataLen + 256/8;
			CMBlock buf(bufLen);

			memcpy(buf, data, dataLen);
			BRSHA256_2(&buf[dataLen], data, dataLen);
			std::string str = Encode(buf, dataLen + 4);

			buf.Zero();

			return str;
		}

		// returns the number of bytes written to data, or total dataLen needed if data is NULL
		CMBlock Base58::CheckDecode(const std::string &str) {
			size_t len;
			uint8_t md[256/8];

			CMBlock buf = Decode(str);

			len = buf.GetSize();
			if (len >= 4) {
				len -= 4;
				BRSHA256_2(md, buf, len);
				if (memcmp(&buf[len], md, sizeof(uint32_t)) != 0) { // verify checksum
					buf.Clear();
				}
				buf.Resize(len);
			} else {
				buf.Clear();
			}

			return buf;
		}

	}
}