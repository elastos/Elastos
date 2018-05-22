// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// reference by https://blog.csdn.net/fengmm521/article/details/78438668

#include <vector>
#include <assert.h>
#include <string.h>

#include "BRInt.h"

#include "SjclBase64.h"

namespace Elastos {
	namespace SDK {

		std::string _Encode(const unsigned char *Data, size_t DataByte) {
			//编码表
			static const char EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			//返回值
			std::string strEncode;
			unsigned char Tmp[4] = {0};
			size_t LineLength = 0;
			for (size_t i = 0; i < (size_t) (DataByte / 3); i++) {
				Tmp[1] = *Data++;
				Tmp[2] = *Data++;
				Tmp[3] = *Data++;
				strEncode += EncodeTable[Tmp[1] >> 2];
				strEncode += EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
				strEncode += EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
				strEncode += EncodeTable[Tmp[3] & 0x3F];
				if (LineLength += 4, LineLength == 76) {
					strEncode += "\r\n";
					LineLength = 0;
				}
			}
			//对剩余数据进行编码
			size_t Mod = DataByte % 3;
			if (Mod == 1) {
				Tmp[1] = *Data++;
				strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
				strEncode += EncodeTable[((Tmp[1] & 0x03) << 4)];
				strEncode += "==";
			} else if (Mod == 2) {
				Tmp[1] = *Data++;
				Tmp[2] = *Data++;
				strEncode += EncodeTable[(Tmp[1] & 0xFC) >> 2];
				strEncode += EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
				strEncode += EncodeTable[((Tmp[2] & 0x0F) << 2)];
				strEncode += "=";
			}

			return strEncode;
		}

		std::string _Decode(const char *Data, size_t DataByte, size_t &OutByte) {
			//解码表
			static const char DecodeTable[] =
				{
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					62, // '+'
					0, 0, 0,
					63, // '/'
					52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
					0, 0, 0, 0, 0, 0, 0,
					0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
					13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
					0, 0, 0, 0, 0, 0,
					26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
					39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
				};
			//返回值
			std::string strDecode;
			size_t nValue;
			size_t i = 0;
			while (i < DataByte) {
				if (*Data != '\r' && *Data != '\n') {
					nValue = DecodeTable[*Data++] << 18;
					nValue += DecodeTable[*Data++] << 12;
					strDecode += (nValue & 0x00FF0000) >> 16;
					OutByte++;
					if (*Data != '=') {
						nValue += DecodeTable[*Data++] << 6;
						strDecode += (nValue & 0x0000FF00) >> 8;
						OutByte++;
						if (*Data != '=') {
							nValue += DecodeTable[*Data++];
							strDecode += nValue & 0x000000FF;
							OutByte++;
						}
					}
					i += 4;
				} else// 回车换行,跳过
				{
					Data++;
					i++;
				}
			}

			return strDecode;
		}

		std::vector<unsigned char> SjclBase64::toBits(const std::string &base64Str) {
			std::string dec;
			size_t OutByte;

			dec = _Decode(base64Str.c_str(), base64Str.size(), OutByte);
			std::vector<unsigned char> ret;
			ret.resize(dec.size());
			memcpy(ret.data(), dec.c_str(), dec.size());

			return ret;
		}

		std::string SjclBase64::fromBits(const unsigned char *bitArray, size_t length) {
			return _Encode(bitArray, length);
		}
	}
}