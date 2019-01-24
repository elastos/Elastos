// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ParamChecker.h"
#include "Utils.h"
#include "Log.h"
#include "Base64.h"
#include "Base58.h"

#include <SDK/Crypto/Crypto.h>

#include <Core/BRCrypto.h>
#include <Core/BRBase58.h>
#include <Core/BRAddress.h>
#include <Core/BRBIP39Mnemonic.h>

#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <iterator>
#include <assert.h>

namespace Elastos {
	namespace ElaWallet {

		std::string Utils::UInt256ToString(const UInt256 &u256, bool reverse) {
			std::stringstream ss;

			if (!reverse) {
				for (int i = 0; i < sizeof(u256.u8); ++i) {
					ss << (char) _hexc(u256.u8[i] >> 4) << (char) _hexc(u256.u8[i]);
				}
			} else {
				for (int i = sizeof(u256.u8) - 1; i >= 0; --i) {
					ss << (char) _hexc(u256.u8[i] >> 4) << (char) _hexc(u256.u8[i]);
				}
			}

			return ss.str();
		}

		UInt256 Utils::UInt256FromString(const std::string &s, bool reverse) {
			UInt256 result = {0};

			if (s.length() / 2 != sizeof(UInt256)) {
				Log::error("UInt256 convert from string=\"{}\" error", s);
				return result;
			}

			if (!reverse) {
				for (int i = 0; i < sizeof(result.u8); ++i) {
					result.u8[i] = (_hexu((s)[2 * i]) << 4) | _hexu((s)[2 * i + 1]);
				}
			} else {
				for (int i = sizeof(result.u8) - 1; i >= 0; --i) {
					result.u8[sizeof(result.u8) - 1 - i] = (_hexu((s)[2 * i]) << 4) | _hexu((s)[2 * i + 1]);
				}
			}

			return result;
		}

		std::string Utils::UInt168ToString(const UInt168 &u168) {
			std::stringstream ss;

			for (int i = 0; i < sizeof(u168.u8); ++i) {
				ss << (char) _hexc(u168.u8[i] >> 4) << (char) _hexc(u168.u8[i]);
			}

			return ss.str();
		}

		UInt168 Utils::UInt168FromString(const std::string &str) {
			UInt168 result = {0};

			if (str.length() / 2 != sizeof(UInt168)) {
				Log::error("UInt168 convert from string=\"{}\" error", str);
				return result;
			}

			for (int i = 0; i < sizeof(result.u8); ++i) {
				result.u8[i] = (_hexu((str)[2 * i]) << 4) | _hexu((str)[2 * i + 1]);
			}

			return result;
		}

		std::string Utils::UInt128ToString(const UInt128 &u128) {
			std::stringstream ss;

			for (int i = 0; i < sizeof(u128.u8); ++i) {
				ss << (char) _hexc(u128.u8[i] >> 4) << (char) _hexc(u128.u8[i]);
			}

			return ss.str();
		}

		UInt128 Utils::UInt128FromString(const std::string &str) {
			UInt128 result = {0};

			if (str.length() / 2 != sizeof(UInt128)) {
				Log::error("UInt128 From String error: str=\"{}\" ", str);
				return result;
			}

			for (int i = 0; i < sizeof(result.u8); ++i) {
				result.u8[i] = (_hexu((str)[2 * i]) << 4) | _hexu((str)[2 * i + 1]);
			}

			return result;
		}

		CMBlock Utils::decodeHex(const std::string &source) {
			ParamChecker::checkCondition(0 != source.length() % 2, Error::HexString,
										 "Decode hex string fail: length is not even");
			CMBlock target;
			target.Resize(source.length() / 2);

			for (int i = 0; i < target.GetSize(); i++) {
				target[i] = (uint8_t) ((_hexu(source[2 * i]) << 4) | _hexu(source[(2 * i) + 1]));
			}

			return target;
		}

		std::string Utils::encodeHex(const uint8_t *hex, size_t hexLen) {
			std::string str;
			str.reserve(2 * hexLen + 1);

			for (size_t i = 0; i < hexLen; ++i) {
				str += char(_hexc(hex[i] >> 4));
				str += char(_hexc(hex[i]));
			}

			return str;
		}

		std::string Utils::encodeHex(const CMBlock &in) {
			return encodeHex(in, in.GetSize());
		}

		bool Utils::Encrypt(std::string &ctBase64, const std::string &data, const std::string &passwd) {
			CMBlock pt;
			pt.SetMemFixed((const uint8_t *)data.c_str(), data.size());
			return Encrypt(ctBase64, pt, passwd);
		}

		bool Utils::Decrypt(std::string &data, const std::string &ctBase64, const std::string &passwd) {
			CMBlock pt;

			if (!Decrypt(pt, ctBase64, passwd)) {
				Log::error("Decrypt fail");
				return false;
			}

			data = std::string(pt, pt.GetSize());
			return true;
		}

		bool Utils::Encrypt(std::string &ctBase64, const void *data, size_t len, const std::string &passwd) {
			CMBlock buf;
			buf.SetMemFixed(data, len);

			return Encrypt(ctBase64, buf, passwd);
		}

		bool Utils::Encrypt(std::string &ctBase64, const CMBlock &data, const std::string &passwd) {
			std::string saltBase64 = "ZRVja4LFrFY=";
			std::string ivBase64 = "n2JUTJ0/yrLdCDPfIcqAzw==";

			return Crypto::Encrypt(ctBase64, data, passwd, saltBase64, ivBase64, "", true);
		}

		bool Utils::Decrypt(CMBlock &data, const std::string &ctBase64, const std::string &passwd) {
			std::string saltBase64 = "ZRVja4LFrFY=";
			std::string ivBase64 = "n2JUTJ0/yrLdCDPfIcqAzw==";

			return Crypto::Decrypt(data, ctBase64, passwd, saltBase64, ivBase64, "", true);
		}

		std::string Utils::UInt168ToAddress(const UInt168 &u) {
			UInt256 hash = UINT256_ZERO;

			BRSHA256_2(&hash, u.u8, sizeof(u));

			CMBlock data(sizeof(UInt168) + 4);
			memcpy(data, u.u8, sizeof(u));
			memcpy(data + sizeof(u), hash.u8, 4);

			return Base58::Encode(data);
		}

		bool Utils::UInt168FromAddress(UInt168 &u, const std::string &address) {
			CMBlock programHash = Base58::CheckDecode(address);
			if (programHash.GetSize() != sizeof(UInt168)) {
				return false;
			}

			memcpy(u.u8, programHash, programHash.GetSize());

			return true;
		}

		CMBlock Utils::GetRandom(size_t bits) {
			size_t bytes = 0 == bits % 8 ? bits / 8 : bits / 8 + 1;
			CMBlock out(bytes);
			for (size_t i = 0; i < bytes; i++) {
				out[i] = Utils::getRandomByte();
			}
			return out;
		}

		bool Utils::PhraseIsValid(const CMemBlock<char> &phrase, const std::vector<std::string> &WordList) {
			bool out = false;
			if (true == phrase && 0 < WordList.size()) {
				const char *wordList[WordList.size()];
				memset(wordList, 0, sizeof(wordList));
				for (size_t i = 0; i < WordList.size(); i++) {
					wordList[i] = WordList[i].c_str();
				}
				out = 1 == BRBIP39PhraseIsValid(wordList, phrase);
			}
			return out;
		}
	}
}
