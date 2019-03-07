// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Key.h"

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Base58.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRCrypto.h>
#include <Core/BRAddress.h>

#include <cstring>
#include <boost/bind.hpp>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <openssl/ecdsa.h>

namespace Elastos {
	namespace ElaWallet {

		Key::Key() {
			_secret = UINT256_ZERO;
			memset(_pubKey, 0, sizeof(_pubKey));
			_compressed = false;
		}

		Key::Key(const Key &key) {
			operator=(key);
		}

		Key::Key(const UInt256 &secret, bool compressed) {
			SetSecret(secret, compressed);
		}

		Key::~Key() {
			Clean();
		}

		Key &Key::operator=(const Key &key) {
			_secret = key._secret;
			memcpy(_pubKey, key._pubKey, sizeof(_pubKey));
			_compressed = key._compressed;
			return *this;
		}

		bool Key::SetPubKey(const CMBlock &pubKey) {
			memcpy(_pubKey, pubKey, pubKey.GetSize() < sizeof(_pubKey) ? pubKey.GetSize() : sizeof(_pubKey));
			_compressed = (pubKey.GetSize() <= 33);

			assert(pubKey.GetSize() == 0 || PubKeyIsValid(_pubKey, pubKey.GetSize()));

			return true;
		}

		// writes the WIF private key to privKey and returns the number of bytes writen, or pkLen needed if privKey is NULL
		// returns empty std::string() on failure
		std::string Key::PrivKey() const {
			CMBlock data(34);
			std::string privKey;

//			if (secp256k1_ec_seckey_verify(_ctx, _secret.u8)) {
				data[0] = BITCOIN_PRIVKEY;
#if BITCOIN_TESTNET
				data[0] = BITCOIN_PRIVKEY_TEST;
#endif

				UInt256Set(&data[1], _secret);
				if (_compressed)
					data[33] = 0x01;
				privKey = Base58::CheckEncode(data, _compressed ? 34 : 33);
				mem_clean(data, data.GetSize());
//			}

			return privKey;
		}

		CMBlock Key::PubKey() {
			size_t size = _compressed ? 33 : 65;

			return CMBlock(_pubKey, size);
		}

		const UInt256 &Key::GetSecret() const {
			return _secret;
		}

		// assigns secret to key and returns true on success
		bool Key::SetSecret(const UInt256 &secret, bool compressed) {
			memset(_pubKey, 0, sizeof(_pubKey));
			_secret = secret;
			_compressed = compressed;

			GeneratePubKey();

			return true;
		}

		// returns true if privKey is a valid private key
		// supported formats are wallet import format (WIF), mini private key format, or hex string
		bool Key::PrivKeyIsValid(const std::string &privKey) const {
			size_t strLen;
			bool r;

			CMBlock data = Base58::CheckDecode(privKey);
			strLen = privKey.length();

			if (data.GetSize() == 33 || data.GetSize() == 34) { // wallet import format: https://en.bitcoin.it/wiki/Wallet_import_format
#if BITCOIN_TESTNET
				r = (data[0] == BITCOIN_PRIVKEY_TEST);
#else
				r = (data[0] == BITCOIN_PRIVKEY);
#endif
			} else if ((strLen == 30 || strLen == 22) && privKey[0] == 'S') { // mini private key format
				char s[strLen + 2];

				strncpy(s, privKey.c_str(), sizeof(s));
				s[sizeof(s) - 2] = '?';
				BRSHA256(data, s, sizeof(s) - 1);
				mem_clean(s, sizeof(s));
				r = (data[0] == 0);
			} else { // hex encoded key
				r = (strspn(privKey.c_str(), "0123456789ABCDEFabcdef") == 64);
			}

			mem_clean(data, data.GetSize());
			return r;
		}

		// assigns privKey to key and returns true on success
		// privKey must be wallet import format (WIF), mini private key format, or hex string
		bool Key::SetPrivKey(const std::string &privKey) {
			uint8_t version = BITCOIN_PRIVKEY;
			bool r = false;

#if BITCOIN_TESTNET
			version = BITCOIN_PRIVKEY_TEST;
#endif

			// mini private key format
			if ((privKey.length() == 30 || privKey.length() == 22) && privKey[0] == 'S') {
				if (! PrivKeyIsValid(privKey)) return 0;
				UInt256 secret;
				BRSHA256(&secret, privKey.c_str(), privKey.length());
				r = SetSecret(secret, 0);
				mem_clean(&secret, sizeof(secret));
			} else {
				CMBlock data = Base58::CheckDecode(privKey);
				if (data.GetSize() == 0 || data.GetSize() == 28) {
					data = Base58::Decode(privKey);
				}

				if (data.GetSize() < sizeof(UInt256) || data.GetSize() > sizeof(UInt256) + 2) { // treat as hex string
					data = Utils::decodeHex(privKey);
				}

				if ((data.GetSize() == sizeof(UInt256) + 1 || data.GetSize() == sizeof(UInt256) + 2) && data[0] == version) {
					r = SetSecret(*(UInt256 *)&data[1], (data.GetSize() == sizeof(UInt256) + 2));
				} else if (data.GetSize() == sizeof(UInt256)) {
					r = SetSecret(*(UInt256 *)data, 0);
				}
				mem_clean(data, data.GetSize());
			}

			return r;
		}

		SignType Key::PrefixToSignType(Prefix prefix) const {
			SignType type;

			switch (prefix) {
				case PrefixStandard:
				case PrefixDeposit:
					type = SignTypeStandard;
					break;
				case PrefixCrossChain:
					type = SignTypeCrossChain;
					break;
				case PrefixMultiSign:
					type = SignTypeMultiSign;
					break;
				case PrefixIDChain:
					type = SignTypeIDChain;
					break;
				case PrefixDestroy:
					type = SignTypeDestroy;
					break;
				default:
					type = SignTypeInvalid;
					break;
			}

			return type;
		}

		/*
		 * m / n
		 * n = pubKeys.size()
		 */
		CMBlock Key::MultiSignRedeemScript(uint8_t m, const std::vector<CMBlock> &pubKeys) {
			std::vector<CMBlock> uniqueSigners;

			for (size_t i = 0; i < pubKeys.size(); ++i) {
				size_t j;
				for (j = 0; j < uniqueSigners.size(); ++j) {
					if (pubKeys[i] == uniqueSigners[j])
						break;
				}

				if (j >= uniqueSigners.size()) {
					uniqueSigners.push_back(pubKeys[i]);
				}
			}

			ParamChecker::checkCondition(uniqueSigners.size() < m, Error::MultiSignersCount,
										 "Required sign count greater than signers");

			ParamChecker::checkCondition(uniqueSigners.size() > sizeof(uint8_t) - OP_1, Error::MultiSignersCount,
										 "Signers should less than 205.");

			std::vector<CMBlock> sortedSigners;
			std::for_each(uniqueSigners.begin(), uniqueSigners.end(), [&sortedSigners](const CMBlock &pubKey) {
				sortedSigners.push_back(pubKey);
			});

			std::sort(sortedSigners.begin(), sortedSigners.end(),
					  boost::bind(&Key::Compare, this, _1, _2));

			ByteStream stream;
			stream.writeUint8(uint8_t(OP_1 + m - 1));
			for (size_t i = 0; i < sortedSigners.size(); i++) {
				stream.writeUint8(uint8_t(sortedSigners[i].GetSize()));
				stream.writeBytes(sortedSigners[i], sortedSigners[i].GetSize());
			}

			stream.writeUint8(uint8_t(OP_1 + sortedSigners.size() - 1));
			stream.writeUint8(SignTypeMultiSign);

			return stream.getBuffer();
		}

		CMBlock Key::RedeemScript(Prefix prefix) const {
			uint8_t size = (uint8_t)(_compressed ? 33 : 65);

			ByteStream stream(size + 2);

			stream.writeUint8(size);
			stream.writeBytes(_pubKey, size);
			stream.writeUint8(PrefixToSignType(prefix));

			return stream.getBuffer();
		}

		UInt168 Key::CodeToProgramHash(Prefix prefix, const CMBlock &code) {
			UInt168 programHash;
			BRHash160(&programHash.u8[1], code, code.GetSize());
			programHash.u8[0] = prefix;

			return programHash;
		}

		UInt168 Key::ProgramHash(Prefix prefix) {
			CMBlock code = RedeemScript(prefix);

			UInt168 programHash;
			BRHash160(&programHash.u8[1], code, code.GetSize());
			programHash.u8[0] = prefix;

			return programHash;
		}

		std::string Key::GetAddress(Prefix prefix) const {
			CMBlock redeemScript = RedeemScript(prefix);
			UInt168 programHash = CodeToProgramHash(prefix, redeemScript);
			return Utils::UInt168ToAddress(programHash);
		}

		CMBlock Key::Sign(const UInt256 &md) const {
			CMBlock signedData;
			signedData.Resize(64);
			memset(signedData, 0, signedData.GetSize());

			EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			if (key) {
				BIGNUM *bnPrivKey = BN_bin2bn((const unsigned char *) &_secret, sizeof(_secret), nullptr);
				if (bnPrivKey) {
					if (1 == EC_KEY_set_private_key(key, bnPrivKey)) {
						ECDSA_SIG *sig = ECDSA_do_sign((unsigned char *)&md, sizeof(md), key);
						if (nullptr != sig) {
							const BIGNUM *r = nullptr;
							const BIGNUM *s = nullptr;
							ECDSA_SIG_get0(sig, &r, &s);
							if (BN_num_bits(r) <= 256 && BN_num_bits(s) <= 256) {
								uint8_t arrBin[256];
								int len = BN_bn2bin(r, arrBin);
								memcpy(&signedData[32 - len], arrBin, len);
								len = BN_bn2bin(s, arrBin);
								memcpy(&signedData[32 + (32 - len)], arrBin, len);
							}
							ECDSA_SIG_free(sig);
						}
					}
					BN_free(bnPrivKey);
				}
				EC_KEY_free(key);
			}

			if (signedData.GetSize() == 0) {
				Log::error("Sign error");
			}

			return signedData;
		}

		CMBlock Key::Sign(const std::string &message) const {
			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());
			return Sign(md);
		}

		CMBlock Key::Sign(const CMBlock &message) const {
			UInt256 md;
			BRSHA256(&md, message, message.GetSize());
			return Sign(md);
		}

		bool Key::Verify(const std::string &message, const CMBlock &signature) const {
			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());

			return Verify(md, signature);
		}

		bool Key::Verify(const UInt256 &md, const CMBlock &signature) const {
			int pubKeyLen = _compressed ? 33 : 65;
			bool result = false;

			if (signature.GetSize() != 64) {
				Log::error("Key verify error: signed data is not 64 bytes");
				return false;
			}

			BIGNUM *bnPubkey = BN_bin2bn(_pubKey, pubKeyLen, nullptr);
			if (bnPubkey == nullptr) {
				Log::error("Key verify error: get pubKey BN error");
				return result;
			}

			EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			if (key != nullptr) {
				const EC_GROUP *curve = EC_KEY_get0_group(key);
				EC_POINT *ecPoint = EC_POINT_bn2point(curve, bnPubkey, nullptr, nullptr);
				if (nullptr != ecPoint) {
					if (1 == EC_KEY_set_public_key(key, ecPoint)) {
						ECDSA_SIG *sig = ECDSA_SIG_new();
						if (nullptr != sig) {
							BIGNUM *r = BN_bin2bn(&signature[0], 32, nullptr);
							BIGNUM *s = BN_bin2bn(&signature[32], 32, nullptr);
							ECDSA_SIG_set0(sig, r, s);
							if (1 == ECDSA_do_verify((uint8_t *)&md, sizeof(md), sig, key)) {
								result = true;
							}
							ECDSA_SIG_free(sig);
						}
					}
					EC_POINT_free(ecPoint);
				}
				EC_KEY_free(key);
			}

			BN_free(bnPubkey);

			return result;
		}

		bool Key::Valid() const {
			return PubKeyIsValid(_pubKey, _compressed ? 33 : 65);
		}

		bool Key::PubKeyIsValid(const void *pubKey, size_t len) {
			bool valid = false;

			EC_POINT *pnt = nullptr;

			BIGNUM *bnp = BN_bin2bn((const unsigned char *)pubKey, (int)len, nullptr);
			if (bnp) {
				EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
				if (key) {
					EC_GROUP *curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
					if (curve) {
						pnt = EC_POINT_bn2point(curve, bnp, nullptr, nullptr);
						if (pnt) {
							if (EC_KEY_set_public_key(key, pnt) && EC_KEY_check_key(key)) {
								valid = true;
							}
							EC_POINT_free(pnt);
						}
						EC_GROUP_free(curve);
					}

					EC_KEY_free(key);
				}
				BN_free(bnp);
			}

			if (!pnt) {
				Log::error("PubKey to point error");
			}

			return valid;
		}

		bool Key::PubKeyEmpty() const {
			uint8_t empty[65];
			memset(empty, 0, sizeof(empty));

			return memcmp(_pubKey, empty, _compressed ? 33 : 65) == 0;
		}

		void Key::GeneratePubKey() const {
			memset(_pubKey, 0, sizeof(_pubKey));
			CMBlock pubKey = BIP32Sequence::PrivKeyToPubKey(_secret);
			if (pubKey.GetSize() != 33 && pubKey.GetSize() != 65) {
				Log::error("Invalid public key length");
			} else {
				memcpy(_pubKey, pubKey, pubKey.GetSize());
			}

			if (pubKey.GetSize() == 33) {
				_compressed = true;
			}
		}

		void Key::Clean() {
			memset(_pubKey, 0, sizeof(_pubKey));
			memset(_secret.u8, 0, sizeof(_secret));
			_compressed = false;
		}

		bool Key::Compare(const CMBlock &a, const CMBlock &b) const {
			BigNum bigIntA = PubKeyDecodePointX(a);
			BigNum bigIntB = PubKeyDecodePointX(b);

			return bigIntA <= bigIntB;
		}

		BigNum Key::PubKeyDecodePointX(const CMBlock &pubKey) const {
			BigNum bigNum;

			if (!PubKeyIsValid(pubKey, pubKey.GetSize())) {
				Log::error("Invalid public key");
				return bigNum;
			}

			EC_POINT *pnt = nullptr;

			BIGNUM *bnp = BN_bin2bn(pubKey, pubKey.GetSize(), nullptr);
			if (bnp) {
				EC_GROUP *curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
				if (curve) {
					pnt = EC_POINT_bn2point(curve, bnp, nullptr, nullptr);
					if (pnt) {
						BIGNUM *x = BN_new();
						BIGNUM *y = BN_new();

						if (1 == EC_POINT_get_affine_coordinates_GFp(curve, pnt, x, y, nullptr)) {
							bigNum = x;
						}
						EC_POINT_free(pnt);
					}
					EC_GROUP_free(curve);
				}
				BN_free(bnp);
			}

			return bigNum;
		}

	}
}
