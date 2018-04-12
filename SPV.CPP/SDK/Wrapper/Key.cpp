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

		Key::Key(std::string privKey) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			assert(!privKey.empty());
			if (setPrivKey(privKey)) {
				Log::error("Failed to set PrivKey");
			}
		}

		Key::Key(ByteData privKey) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			char data[privKey.length + 1];
			memcpy((void *) data, (void *) privKey.data, sizeof(uint8_t) * privKey.length);
			if (setPrivKey(data)) {
				Log::error("Failed to set PrivKey");
			}
		}

		Key::Key(UInt256 secret, bool compressed) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			if (!setSecret(secret, compressed)) {
				Log::error("Failed to set Sercret");
			}
		}

		Key::Key(ByteData seed, uint32_t chain, uint32_t index) {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			BRBIP32PrivKey(_key.get(), seed.data, seed.length, chain, index);
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

		ByteData Key::getPubkey() const {
			return ByteData(_key->pubKey, 65);
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

		ByteData Key::getSeedFromPhrase(ByteData phrase) {
			UInt512 *key = new UInt512;
			char *charPhrase = (char *) phrase.data;
			BRBIP39DeriveKey(key->u8, charPhrase, nullptr);
			return ByteData(key->u8, sizeof(UInt512));
		}

		ByteData Key::getAuthPrivKeyForAPI(ByteData seed) {
			BRKey *key = new BRKey;
			BRBIP32APIAuthKey(key, (void *) &seed, seed.length);
			char rawKey[BRKeyPrivKey(key, nullptr, 0)];
			BRKeyPrivKey(key, rawKey, sizeof(rawKey));
			return ByteData((uint8_t *) &rawKey, sizeof(rawKey));
		}

		std::string Key::getAuthPublicKeyForAPI(ByteData privKey) {
			BRKey key;
			BRKeySetPrivKey(&key, (const char *) privKey.data);
			size_t len = BRKeyPubKey(&key, nullptr, 0);
			uint8_t pubKey[len];
			BRKeyPubKey(&key, &pubKey, len);
			size_t strLen = BRBase58Encode(nullptr, 0, pubKey, len);
			char base58string[strLen];
			BRBase58Encode(base58string, strLen, pubKey, len);

			return base58string;
		}

		std::string Key::decryptBip38Key(std::string privKey, std::string pass) {
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

		bool Key::setPrivKey(std::string privKey) {
			return BRKeySetPrivKey(_key.get(), privKey.c_str()) != 0;
		}

		bool Key::setSecret(UInt256 secret, bool compressed) {
			return BRKeySetSecret(_key.get(), (const UInt256 *) &secret, compressed) != 0;
		}

		ByteData Key::compactSign(ByteData data) const {
			UInt256 md32;
			UInt256Get(&md32, data.data);
			size_t sigLen = BRKeyCompactSign(_key.get(), nullptr, 0, md32);
			uint8_t *compactSig = new uint8_t[sigLen];
			sigLen = BRKeyCompactSign(_key.get(), compactSig, sigLen, md32);
			return ByteData(compactSig, sigLen);
		}

		ByteData Key::encryptNative(ByteData data, ByteData nonce) const {
			uint8_t *out = new uint8_t[16 + data.length];
			size_t outSize = BRChacha20Poly1305AEADEncrypt(out, 16 + data.length, _key.get(), nonce.data,
														   data.data, data.length, nullptr, 0);
			return ByteData(out, outSize);
		}

		ByteData Key::decryptNative(ByteData data, ByteData nonce) const {
			uint8_t *out = new uint8_t[data.length];
			size_t outSize = BRChacha20Poly1305AEADDecrypt(out, data.length, _key.get(), nonce.data,
														   data.data, data.length, nullptr, 0);
			return ByteData(out, outSize);
		}


		std::string Key::address() const {
			BRAddress address = BR_ADDRESS_NONE;
			BRKeyAddress(_key.get(), address.s, sizeof(address));
			assert(address.s[0] != '\0');

			return address.s;
		}

		ByteData Key::sign(UInt256 messageDigest) const {
			uint8_t *signature = new uint8_t[256];
			size_t signatureLen = BRKeySign(_key.get(), signature, 256, messageDigest);
			assert (signatureLen <= 256);
			return ByteData(signature, signatureLen);
		}

		bool Key::verify(UInt256 messageDigest, ByteData signature) const {
			return BRKeyVerify(_key.get(), messageDigest, signature.data, signature.length) == 1;
		}

		bool Key::isValidBitcoinPrivateKey(std::string key) {
			return BRPrivKeyIsValid(key.c_str()) == 1;
		}

		bool Key::isValidBitcoinBIP38Key(std::string key) {
			return BRBIP38KeyIsValid(key.c_str()) == 1;
		}

		std::string Key::encodeHex(ByteData in) {
			char *dataHex = Utils::encodeHexCreate(nullptr, in.data, in.length);
			return dataHex;
		}

		ByteData Key::decodeHex(std::string s) {
			size_t dataLen = 0;
			char *str = const_cast<char *>(s.c_str());
			uint8_t *data = Utils::decodeHexCreate(&dataLen, str, strlen(str));
			return ByteData(data, dataLen);
		}

		UInt256 Key::encodeSHA256(std::string message) {
			UInt256 md;
			BRSHA256((void *) &md, (void *) &message, strlen(message.c_str()));
			return md;
		}


	}
}