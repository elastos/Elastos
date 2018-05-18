// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string.h>

#include "BRBIP39Mnemonic.h"
#include "BRBIP32Sequence.h"
#include "BRBase58.h"
#include "BRBIP38Key.h"
#include "BRCrypto.h"
#include "BRAddress.h"
#include "BRKey.h"

#include "Common/Utils.h"
#include "Log.h"
#include "Key.h"

namespace Elastos {
	namespace SDK {

		Key::Key() {
			_key = boost::shared_ptr<BRKey>(new BRKey);
		}

		Key::Key(const boost::shared_ptr<BRKey> &brkey) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			*_key = *brkey;
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

		Key::Key(const CMBlock &seed, uint32_t chain, uint32_t index) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			BRBIP32PrivKey(_key.get(), seed, seed.GetSize(), chain, index);
		}

		std::string Key::toString() const {
			//todo complete me
			return "";
		}

		BRKey *Key::getRaw() const {
			return _key.get();
		}

		UInt256 Key::getSecret() const {
			return _key->secret;
		}

		CMBlock Key::getPubkey() const {
			CMBlock ret(65);
			memcpy(ret, _key->pubKey, 65);

			return ret;
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

		CMBlock Key::getSeedFromPhrase(const CMBlock &phrase) {
			UInt512 key;
			const char *charPhrase = (char *)(void *)phrase;
			BRBIP39DeriveKey(key.u8, charPhrase, nullptr);

			CMBlock ret(sizeof(UInt512));
			memcpy(ret, key.u8, sizeof(UInt512));
		}

		CMBlock Key::getAuthPrivKeyForAPI(const CMBlock &seed) {
			BRKey key;
			BRBIP32APIAuthKey(&key, (void *)seed, seed.GetSize());
			char rawKey[BRKeyPrivKey(&key, nullptr, 0)];
			BRKeyPrivKey(&key, rawKey, sizeof(rawKey));

			CMBlock ret(sizeof(rawKey));
			memcpy(ret, &rawKey, sizeof(rawKey));

			return ret;
		}

		std::string Key::getAuthPublicKeyForAPI(const CMBlock &privKey) {
			BRKey key;
			BRKeySetPrivKey(&key, (const char *)(void *)privKey);
			size_t len = BRKeyPubKey(&key, nullptr, 0);
			uint8_t pubKey[len];
			BRKeyPubKey(&key, &pubKey, len);
			size_t strLen = BRBase58Encode(nullptr, 0, pubKey, len);
			char base58string[strLen];
			BRBase58Encode(base58string, strLen, pubKey, len);

			return base58string;
		}

		std::string Key::decryptBip38Key(const std::string &privKey, const std::string &pass) {
			BRKey key;
			int result = BRKeySetBIP38Key(&key, privKey.c_str(), pass.c_str());

			if (result) {
				char pk[BRKeyPrivKey(&key, NULL, 0)];

				BRKeyPrivKey(&key, pk, sizeof(pk));
				return pk;
			} else {
				return "";
			}
		}

		bool Key::setPrivKey(const std::string &privKey) {
			return BRKeySetPrivKey(_key.get(), privKey.c_str()) != 0;
		}

		bool Key::setSecret(const UInt256 &secret, bool compressed) {
			return BRKeySetSecret(_key.get(), (const UInt256 *) &secret, compressed) != 0;
		}

		CMBlock Key::compactSign(const CMBlock &data) const {
			UInt256 md32;
			UInt256Get(&md32, data);
			size_t sigLen = BRKeyCompactSign(_key.get(), nullptr, 0, md32);
			CMBlock compactSig(sigLen);
			sigLen = BRKeyCompactSign(_key.get(), compactSig, sigLen, md32);

			return compactSig;
		}

		CMBlock Key::encryptNative(const CMBlock &data, const CMBlock &nonce) const {
			CMBlock out(16 + data.GetSize());
			size_t outSize = BRChacha20Poly1305AEADEncrypt(out, 16 + data.GetSize(), _key.get(), nonce,
														   data, data.GetSize(), nullptr, 0);
			return out;
		}

		CMBlock Key::decryptNative(const CMBlock &data, const CMBlock &nonce) const {
			CMBlock out(data.GetSize());
			size_t outSize = BRChacha20Poly1305AEADDecrypt(out, data.GetSize(), _key.get(), nonce,
														   data, data.GetSize(), nullptr, 0);
			return out;
		}


		std::string Key::address() const {
			BRAddress address = BR_ADDRESS_NONE;
			BRKeyAddress(_key.get(), address.s, sizeof(address));
			assert(address.s[0] != '\0');

			return address.s;
		}

		CMBlock Key::sign(const UInt256 &messageDigest) const {
			CMBlock signature(256);
			size_t signatureLen = BRKeySign(_key.get(), signature, 256, messageDigest);
			assert (signatureLen <= 256);

			return signature;
		}

		bool Key::verify(const UInt256 &messageDigest, const CMBlock &signature) const {
			return BRKeyVerify(_key.get(), messageDigest, signature, signature.GetSize()) == 1;
		}

		bool Key::isValidBitcoinPrivateKey(const std::string &key) {
			return BRPrivKeyIsValid(key.c_str()) == 1;
		}

		bool Key::isValidBitcoinBIP38Key(const std::string &key) {
			return BRBIP38KeyIsValid(key.c_str()) == 1;
		}

		std::string Key::encodeHex(const CMBlock &in) {
			char *dataHex = Utils::encodeHexCreate(nullptr, in, in.GetSize());
			return dataHex;
		}

		CMBlock Key::decodeHex(const std::string &s) {
			size_t dataLen = 0;
			char *str = const_cast<char *>(s.c_str());
			uint8_t *data = Utils::decodeHexCreate(&dataLen, str, strlen(str));

			CMBlock ret;
			ret.SetMem(data, dataLen);

			return ret;
		}

		UInt256 Key::encodeSHA256(const std::string &message) {
			UInt256 md;
			BRSHA256((void *) &md, (void *) message.c_str(), strlen(message.c_str()));
			return md;
		}


	}
}