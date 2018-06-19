// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <stdlib.h>
#include <Core/BRCrypto.h>
#include <algorithm>
#include <iterator>

#include "assert.h"
#include "Utils.h"
#include "AES_256_CCM.h"
#include "BTCBase58.h"
#include "Base64.h"
#include "BRAddress.h"
#include "Transaction.h"
#include "Log.h"

namespace Elastos {
	namespace ElaWallet {

		std::string Utils::UInt256ToString(const UInt256 &u256) {
			std::stringstream ss;

			for (int i = 0; i < sizeof(u256.u8); ++i) {
				ss << (char) _hexc(u256.u8[i] >> 4) << (char) _hexc(u256.u8[i]);
			}

			return ss.str();
		}

		UInt256 Utils::UInt256FromString(const std::string &s) {
			UInt256 result = {0};

			if (s.length() / 2 != sizeof(UInt256)) {
				Log::getLogger()->error("UInt256 convert from string=\"{}\" error", s);
				return result;
			}

			for (int i = 0; i < sizeof(result.u8); ++i) {
				result.u8[i] = (_hexu((s)[2 * i]) << 4) | _hexu((s)[2 * i + 1]);
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
				Log::getLogger()->error("UInt168 convert from string=\"{}\" error", str);
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
				Log::getLogger()->error("UInt128 From String error: str=\"{}\" ", str);
				return result;
			}

			for (int i = 0; i < sizeof(result.u8); ++i) {
				result.u8[i] = (_hexu((str)[2 * i]) << 4) | _hexu((str)[2 * i + 1]);
			}

			return result;
		}

		void Utils::decodeHex(uint8_t *target, size_t targetLen, const char *source, size_t sourceLen) {
			if (2 * targetLen < sourceLen || 0 != sourceLen % 2) {
				Log::getLogger()->error("decodeHex error: targetLen={}, sourceLen={}", targetLen, sourceLen);
				return;
			}

			for (int i = 0; i < targetLen; i++) {
				target[i] = (uint8_t) ((_hexu(source[2 * i]) << 4) | _hexu(source[(2 * i) + 1]));
			}
		}

		size_t Utils::decodeHexLength(size_t stringLen) {
			assert (0 == stringLen % 2);
			return stringLen / 2;
		}

		uint8_t *Utils::decodeHexCreate(size_t *targetLen, char *source, size_t sourceLen) {
			size_t length = decodeHexLength(sourceLen);
			if (nullptr != targetLen) *targetLen = length;
			uint8_t *target = (uint8_t *) malloc(length);
			decodeHex(target, length, source, sourceLen);
			return target;
		}

		void Utils::encodeHex(char *target, size_t targetLen, const uint8_t *source, size_t sourceLen) {
			assert (targetLen >= 2 * sourceLen + 1);

			for (int i = 0; i < sourceLen; i++) {
				target[2 * i] = (uint8_t) _hexc (source[i] >> 4);
				target[2 * i + 1] = (uint8_t) _hexc (source[i]);
			}
			target[2 * sourceLen] = '\0';
		}

		std::string Utils::encodeHex(const uint8_t *hex, size_t hexLen) {
			if (hexLen == 0) {
				return std::string();
			}

			char buf[hexLen * 2 + 1];
			encodeHex(buf, hexLen * 2 + 1, hex, hexLen);
			std::string str(buf);

			return str;
		}

		size_t Utils::encodeHexLength(size_t byteArrayLen) {
			return 2 * byteArrayLen + 1;
		}

		std::string Utils::encodeHexCreate(size_t *targetLen, uint8_t *source, size_t sourceLen) {
			size_t length = encodeHexLength(sourceLen);
			if (nullptr != targetLen) *targetLen = length;
			char target[length];
			encodeHex(target, length, source, sourceLen);
			return std::string(target, length - 1);
		}

		UInt128 Utils::generateRandomSeed() {
			UInt128 result;
			for (size_t i = 0; i < sizeof(result); ++i) {
				result.u8[i] = getRandomByte();
			}
			return result;
		}

		CMBlock Utils::encrypt(const CMBlock &data, const std::string &password) {
			static unsigned char iv[] = {0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2, 0xDD, 0x08, 0x33, 0xDF, 0x21,
										 0xCA, 0x80,
										 0xCF};
			static unsigned char salt[] = {0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56};
			CMBlock _iv, _salt;
			_iv.SetMemFixed(iv, sizeof(iv));
			_salt.SetMemFixed(salt, sizeof(salt));
			return encrypt(data, password, _salt, _iv, true);
		}

		CMBlock Utils::decrypt(const CMBlock &encryptedData, const std::string &password) {
			static unsigned char iv[] = {0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2, 0xDD, 0x08, 0x33, 0xDF, 0x21,
										 0xCA, 0x80,
										 0xCF};
			static unsigned char salt[] = {0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56};
			CMBlock _iv, _salt;
			_iv.SetMemFixed(iv, sizeof(iv));
			_salt.SetMemFixed(salt, sizeof(salt));
			return decrypt(encryptedData, password, _salt, _iv, true);
		}

		CMBlock
		Utils::encrypt(const CMBlock &data, const std::string &password, CMBlock &salt, CMBlock &iv, bool bAes128) {
			CMBlock ret;
			CMBlock enc = AES_256_CCM::encrypt(data, data.GetSize(),
											   (unsigned char *) password.c_str(), password.size(), salt,
											   salt.GetSize(), iv, iv.GetSize(), bAes128);
			if (true == enc) {
				std::string enc_bs64 = Base64::fromBits(enc, enc.GetSize());
				ret.Resize(enc_bs64.size() + 1);
				ret.Zero();
				memcpy(ret, enc_bs64.c_str(), enc_bs64.size());
			}
			return ret;
		}

		CMBlock
		Utils::decrypt(const CMBlock &encryptedData, const std::string &password, CMBlock &salt, CMBlock &iv,
					   bool bAes128) {
			CMBlock ret;
			if (false == encryptedData) {
				return ret;
			}
			std::string enc_str = (const char *) (void *) encryptedData;
			std::vector<unsigned char> enc = Base64::toBits(enc_str);
			ret = AES_256_CCM::decrypt(enc.data(), enc.size(), (unsigned char *) password.c_str(), password.size(),
									   salt, salt.GetSize(), iv, iv.GetSize(), bAes128);
			return ret;
		}

		std::string Utils::UInt168ToAddress(const UInt168 &u) {
			UInt256 hash = UINT256_ZERO;
			size_t uSize = sizeof(UInt168);

			BRSHA256_2(&hash, u.u8, uSize);

			size_t dataLen = uSize + 4;
			uint8_t data[sizeof(UInt168) + 4] = {0};
			memcpy(data, u.u8, uSize);
			memcpy(data + uSize, hash.u8, 4);

			return BTCBase58::EncodeBase58(data, dataLen);
		}

		bool Utils::UInt168FromAddress(UInt168 &u, const std::string &address) {
			return 0 != BRAddressHash168(&u, address.c_str());
		}

		uint32_t Utils::getAddressTypeBySignType(const int signType) {
			if (signType == ELA_STANDARD) {
				return ELA_STAND_ADDRESS;
			} else if (signType == ELA_MULTISIG) {
				return ELA_MULTISIG_ADDRESS;
			} else if (signType == ELA_CROSSCHAIN) {
				return ELA_CROSSCHAIN_ADDRESS;
			} else if (signType == ELA_IDCHAIN) {
				return ELA_IDCHAIN_ADDRESS;
			} else {
				throw std::logic_error("error signType.");
			}
		}

		UInt168 Utils::codeToProgramHash(const std::string &redeemScript) {
			size_t scriptLen = 0;
			uint8_t *script = Utils::decodeHexCreate(&scriptLen, (char *)(redeemScript.c_str()), redeemScript.size());
			CMBlock redeedScript;
			redeedScript.SetMem(script, scriptLen);

			return codeToProgramHash(redeedScript);
		}

		UInt168 Utils::codeToProgramHash(const CMBlock &redeemScript) {
			UInt160 hash = UINT160_ZERO;
			size_t len = redeemScript.GetSize();
			BRHash160(&hash, redeemScript, len);
			int signType = redeemScript[len - 1];
			uint32_t addressType = Utils::getAddressTypeBySignType(signType);

			UInt168 uInt168 = UINT168_ZERO;
			memcpy(&uInt168.u8[1], &hash.u8[0], sizeof(hash.u8));
			uInt168.u8[0] = addressType;
			return uInt168;
		}

		std::string Utils::encodeHex(const CMBlock &in) {
			return encodeHexCreate(nullptr, in, in.GetSize());
		}

		CMBlock Utils::decodeHex(const std::string &s) {
			size_t dataLen = 0;
			char *str = const_cast<char *>(s.c_str());
			uint8_t *data = decodeHexCreate(&dataLen, str, strlen(str));

			CMBlock ret;
			ret.SetMem(data, dataLen);

			return ret;
		}
	}
}
