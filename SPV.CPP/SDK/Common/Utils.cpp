// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <stdlib.h>
#include <Core/BRCrypto.h>
#include <algorithm>
#include <iterator>
#include <Core/BRBase58.h>
#include <Core/BRAddress.h>

#include "assert.h"
#include "Utils.h"
#include "AES_256_CCM.h"
#include "Base64.h"
#include "BRAddress.h"
#include "Log.h"
#include "ParamChecker.h"

namespace Elastos {
	namespace ElaWallet {

		std::string Utils::UInt256ToString(const UInt256 &u256, bool reverse) {
			std::stringstream ss;

			if(!reverse) {
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
				Log::getLogger()->error("UInt256 convert from string=\"{}\" error", s);
				return result;
			}

			if(!reverse) {
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

		bool Utils::Encrypt(std::string &ctBase64, const CMBlock &data, const std::string &passwd) {
			std::string saltBase64 = "ZRVja4LFrFY=";
			std::string ivBase64 = "n2JUTJ0/yrLdCDPfIcqAzw==";

			return AES_256_CCM::Encrypt(ctBase64, data, passwd, saltBase64, ivBase64, "", true);
		}

		bool Utils::Decrypt(CMBlock &data, const std::string &ctBase64, const std::string &passwd) {
			std::string saltBase64 = "ZRVja4LFrFY=";
			std::string ivBase64 = "n2JUTJ0/yrLdCDPfIcqAzw==";

			return AES_256_CCM::Decrypt(data, ctBase64, passwd, saltBase64, ivBase64, "", true);
		}

		std::string Utils::UInt168ToAddress(const UInt168 &u) {
			UInt256 hash = UINT256_ZERO;
			size_t uSize = sizeof(UInt168);

			BRSHA256_2(&hash, u.u8, uSize);

			size_t dataLen = uSize + 4;
			uint8_t data[sizeof(UInt168) + 4] = {0};
			memcpy(data, u.u8, uSize);
			memcpy(data + uSize, hash.u8, 4);

			BRAddress result;
			BRBase58Encode(result.s, sizeof(result.s), data, dataLen);
			return result.s;
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
			} else if (signType == ELA_DESTROY) {
				return ELA_DESTROY_ADDRESS;
			} else {
				ParamChecker::checkCondition(true, Error::SignType, "Unknown sign type");
			}
			return 0;
		}

		UInt168 Utils::codeToProgramHash(const std::string &redeemScript) {
			return codeToProgramHash(Utils::decodeHex(redeemScript));
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

	}
}
