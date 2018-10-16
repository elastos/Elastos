// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>
#include <secp256k1.h>
#include <Core/BRKey.h>

#include "BRBIP39Mnemonic.h"
#include "BRBIP32Sequence.h"
#include "BRBase58.h"
#include "BRBIP38Key.h"
#include "BRCrypto.h"
#include "BRAddress.h"

#include "Common/Utils.h"
#include "Log.h"
#include "Key.h"
#include "ByteStream.h"
#include "SDK/Transaction/Transaction.h"
#include "ParamChecker.h"
#include "BigIntFormat.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		Key::Key() {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			memset(_key.get(), 0, sizeof(BRKey));
		}

		Key::Key(BRKey *brkey) {
			_key = boost::shared_ptr<BRKey>(brkey);
		}

		Key::Key(const BRKey &brkey) {
			_key = boost::shared_ptr<BRKey>(new BRKey());
			*_key = brkey;
		}

		Key::Key(const std::string &privKey) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			assert(!privKey.empty());
			if (!setPrivKey(privKey)) {
				Log::error("Failed to set PrivKey");
			}
		}

		Key::Key(const CMBlock &privKey) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			char data[privKey.GetSize() + 1];
			memcpy((void *) data, (void *) privKey, sizeof(uint8_t) * privKey.GetSize());
			if (!setPrivKey(data)) {
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
			CMBlock privKey;
			privKey.SetMemFixed((uint8_t *) &_key->secret, sizeof(_key->secret));
			CMBlock result;
			result.Resize(33);
			getPubKeyFromPrivKey(result, (UInt256 *) (uint8_t *) privKey);
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

		std::string Key::getPrivKey() const {
			size_t privKeyLen = (size_t) BRKeyPrivKey(_key.get(), nullptr, 0);
			char privKey[privKeyLen + 1];
			BRKeyPrivKey(_key.get(), privKey, privKeyLen);
			privKey[privKeyLen] = '\0';
			return privKey;
		}

		bool Key::setPrivKey(const std::string &privKey) {
			bool ret = BRKeySetPrivKey(_key.get(), privKey.c_str()) != 0;
			if (ret) {
				setPublicKey();
			}
			return ret;
		}

		bool Key::setSecret(const UInt256 &secret, bool compressed) {
			bool ret = BRKeySetSecret(_key.get(), &secret, compressed) != 0;
			if (ret) {
				setPublicKey();
			}
			return ret;
		}

		void Key::setPublicKey() {
			UInt256 secret = getSecret();
			ParamChecker::checkCondition(0 != UInt256IsZero(&secret), Error::Key,
										 "Secret is zero, can't generate publicKey");
			CMBlock secretData;
			secretData.SetMemFixed(secret.u8, sizeof(secret));

			CMBlock pubKey;
			pubKey.Resize(33);
			getPubKeyFromPrivKey(pubKey, (UInt256 *) (uint8_t *) secretData);

			memset(_key->pubKey, 0, sizeof(_key->pubKey));
			memcpy(_key->pubKey, pubKey, pubKey.GetSize());
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

		bool Key::verify(const UInt256 &messageDigest, const CMBlock &signature) const {
			CMBlock publicKey = getPubkey();
			CMemBlock<char> mbcPubKey = Hex2Str(publicKey);
			std::string pubKey = (const char *) mbcPubKey;

			return Key::verifyByPublicKey(pubKey, messageDigest, signature);
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
