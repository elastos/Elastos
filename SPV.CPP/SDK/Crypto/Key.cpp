// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Key.h"

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Common/ParamChecker.h>

#include <Core/BRKey.h>
#include <Core/BRBIP39Mnemonic.h>
#include <Core/BRBIP32Sequence.h>
#include <Core/BRBase58.h>
#include <Core/BRBIP38Key.h>
#include <Core/BRCrypto.h>
#include <Core/BRAddress.h>

#include <secp256k1.h>
#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		Key::Key() {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			memset(_key.get(), 0, sizeof(BRKey));
		}

		Key::Key(BRKey *brkey) {
			_key = boost::shared_ptr<BRKey>(brkey);

			getPubKeyFromPrivKey(_key->pubKey, sizeof(_key->pubKey), &_key->secret);
		}

		Key::Key(const BRKey &brkey) {
			_key = boost::shared_ptr<BRKey>(new BRKey());
			*_key = brkey;

			getPubKeyFromPrivKey(_key->pubKey, sizeof(_key->pubKey), &_key->secret);
		}

		Key::Key(const std::string &privKey) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			assert(!privKey.empty());
			if (!setPrivKey(privKey)) {
				Log::error("Failed to set PrivKey");
			}
		}

		Key::Key(const UInt256 &secret, bool compressed) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			if (!setSecret(secret, compressed)) {
				Log::error("Failed to set Sercret");
			}
		}

		std::string Key::toString() const {
			return Utils::UInt256ToString(_key->secret);
		}

		BRKey *Key::getRaw() const {
			return _key.get();
		}

		UInt256 Key::getSecret() const {
			return _key->secret;
		}

		CMBlock Key::getPubkey() const {
			CMBlock result;
			int len = getPubKeyFromPrivKey(_key->pubKey, sizeof(_key->pubKey), &_key->secret);
			if (len != 33 && len != 65) {
				Log::error("Invalid public key length");
				return result;
			}

			result.SetMemFixed(_key->pubKey, len);

			return result;
		}

		bool Key::setPubKey(const CMBlock pubKey) {
			ParamChecker::checkCondition(pubKey.GetSize() != 33 && pubKey.GetSize() != 65, Error::PubKeyLength,
										 "Invaid public key length");

			memcpy(_key->pubKey, pubKey, pubKey.GetSize());
			_key->compressed = (pubKey.GetSize() <= 33);
			return true;
		}

		bool Key::getCompressed() const {
			return _key->compressed != 0;
		}

		/*
		 * writes the WIF private key to privKey and returns the number of bytes writen,
		 * or pkLen needed if privKey is NULL
		 * returns 0 on failure
		 */
		std::string Key::getPrivKey() const {
			size_t privKeyLen = (size_t) BRKeyPrivKey(_key.get(), nullptr, 0);
			char privKey[privKeyLen];
			BRKeyPrivKey(_key.get(), privKey, privKeyLen);
			return privKey;
		}

		/*
		 * assigns privKey to key and returns true on success
		 * privKey must be wallet import format (WIF), mini private key format, or hex string
		 */
		bool Key::setPrivKey(const std::string &privKey) {
			if (BRKeySetPrivKey(_key.get(), privKey.c_str()) == 0) {
				Log::error("Invalid privKey");
				return false;
			}

			return 0 != getPubKeyFromPrivKey(_key->pubKey, sizeof(_key->pubKey), &_key->secret);
		}

		bool Key::setSecret(const UInt256 &secret, bool compressed) {
			if (BRKeySetSecret(_key.get(), &secret, compressed) == 0) {
				Log::error("Invalid privKey");
				return false;
			}

			return 0 != getPubKeyFromPrivKey(_key->pubKey, sizeof(_key->pubKey), &_key->secret);
		}

		CMBlock Key::compactSign(const CMBlock &data) const {

			CMBlock signedData;
			signedData.Resize(65);
			ECDSA65Sign_sha256(&_key->secret, sizeof(_key->secret), (const UInt256 *) &data[0], signedData,
							   signedData.GetSize());
			return signedData;
		}

		std::string Key::compactSign(const std::string &message) const {
			CMBlock md(sizeof(UInt256));
			BRSHA256(md, message.c_str(), message.size());

			CMBlock signedData = compactSign(md);
			return Utils::encodeHex(signedData);
		}

		CMBlock Key::encryptNative(const CMBlock &data, const CMBlock &nonce) const {
			CMBlock out(16 + data.GetSize());
			size_t outSize = BRChacha20Poly1305AEADEncrypt(out, 16 + data.GetSize(), _key.get(), nonce,
														   data, data.GetSize(), nullptr, 0);
			return out;
		}

		CMBlock Key::decryptNative(const CMBlock &data, const CMBlock &nonce) const {
			CMBlock out(1024);
			size_t outSize = BRChacha20Poly1305AEADDecrypt(out, data.GetSize(), _key.get(), nonce,
														   data, data.GetSize(), nullptr, 0);
			out.Resize(outSize);
			return out;
		}


		std::string Key::address() const {
			return keyToAddress(ELA_STANDARD);
		}

		std::string Key::keyToAddress(const int signType) const {
			std::string redeedScript = keyToRedeemScript(signType);

			UInt168 hash = Utils::codeToProgramHash(redeedScript);

			std::string address = Utils::UInt168ToAddress(hash);

			return address;
		}

		UInt168 Key::keyToUInt168BySignType(const int signType) {
			UInt168 uInt168 = hashTo168();
			if (signType == ELA_STANDARD) {
				uInt168.u8[0] = ELA_STAND_ADDRESS;
			} else if (signType == ELA_MULTISIG) {
				uInt168.u8[0] = ELA_MULTISIG_ADDRESS;
			} else if (signType == ELA_CROSSCHAIN) {
				uInt168.u8[0] = ELA_CROSSCHAIN_ADDRESS;
			} else if (signType == ELA_IDCHAIN) {
				uInt168.u8[0] = ELA_IDCHAIN_ADDRESS;
			}
			return uInt168;
		}

		const UInt160 Key::hashTo160() {
			UInt160 hash = UINT160_ZERO;
			size_t len = getCompressed() ? 33 : 65;
			CMBlock pubKey = getPubkey();
			BRHash160(&hash, _key->pubKey, len);
			return hash;
		}

		const UInt168 Key::hashTo168() {
			UInt168 hash = UINT168_ZERO;
			size_t len;
			int size = sizeof(hash);
			hash.u8[size - 1] = ELA_STANDARD;
			//todo add publicKey verify
			if (true) {
				BRHash168(&hash, _key->pubKey, len);
			}
			UInt168 uInt168 = UINT168_ZERO;
			uInt168.u8[0] = ELA_STAND_ADDRESS;
			memcpy(&uInt168.u8[1], &hash.u8[0], sizeof(hash.u8) - 1);
			return uInt168;
		}

		bool Key::verifyByPublicKey(const std::string &publicKey, const std::string &message,
									const std::string &signature) {
			CMBlock signatureData = Utils::decodeHex(signature);

			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());

			return verifyByPublicKey(publicKey, md, signatureData);
		}

		bool Key::verifyByPublicKey(const std::string &publicKey, const UInt256 &messageDigest,
									const CMBlock &signature) {
			CMBlock pubKey = Utils::decodeHex(publicKey);
			return ECDSA65Verify_sha256((uint8_t *) (void *) pubKey, pubKey.GetSize(), &messageDigest, signature,
										signature.GetSize()) != 0;
		}

		std::string Key::keyToRedeemScript(int signType) const {
			if (signType == ELA_MULTISIG_ADDRESS) {
				return "";
			}

			uint64_t size = (getCompressed()) ? 33 : 65;

			ByteStream buff(size + 2);

			buff.writeUint8((uint8_t) size);

			buff.writeBytes(_key->pubKey, size);

			buff.writeUint8((uint8_t) signType);

			CMBlock script = buff.getBuffer();
			return Utils::encodeHex(script, script.GetSize());
		}

	}
}
