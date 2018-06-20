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
#include "Transaction.h"
#include "BTCKey.h"
#include "ParamChecker.h"
#include "ELABIP32Sequence.h"

#define BIP32_SEED_KEY "ELA seed"

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
			_key = boost::shared_ptr<BRKey>(new BRKey(brkey));
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
			ELABIP32Sequence::BIP32PrivKey(_key.get(), seed, seed.GetSize(), chain, index);
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
			size_t size = getCompressed() ? 33 : 65;
			CMBlock ret(size);
			memcpy(ret, _key->pubKey, size);

			return ret;
		}

		bool Key::setPubKey(const CMBlock pubKey) {

			if (!BTCKey::PublickeyIsValid(pubKey, NID_X9_62_prime256v1)) {
				return false;
			}
			CMBlock publicKey;
			publicKey.SetMemFixed(pubKey, pubKey.GetSize());
			ParamChecker::checkDataNotEmpty(publicKey, true);
			assert(publicKey.GetSize() == 33 || publicKey.GetSize() == 65);

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

		CMBlock Key::getSeedFromPhrase(const CMBlock &phrase, const std::string &phrasePass) {
			UInt512 key;
			const char *charPhrase = (char *) (void *) phrase;
			const char *charPhrasePass = phrasePass == "" ? nullptr : phrasePass.c_str();
			BRBIP39DeriveKey(key.u8, charPhrase, charPhrasePass);

			CMBlock ret(sizeof(UInt512));
			memcpy(ret, key.u8, sizeof(UInt512));

			return ret;
		}

		CMBlock Key::getAuthPrivKeyForAPI(const CMBlock &seed) {
			BRKey key;
			ELABIP32Sequence::BIP32APIAuthKey(&key, seed, seed.GetSize());
			char rawKey[BRKeyPrivKey(&key, nullptr, 0)];
			BRKeyPrivKey(&key, rawKey, sizeof(rawKey));
			CMBlock ret(sizeof(rawKey));
			memcpy(ret, &rawKey, sizeof(rawKey));

			return ret;
		}

		std::string Key::getAuthPublicKeyForAPI(const CMBlock &privKey) {
			BRKey key;
			BRKeySetPrivKey(&key, (const char *) (void *) privKey);

			CMBlock pubKey;
			CMBlock secret;
			secret.SetMemFixed(key.secret.u8, sizeof(key.secret));
			pubKey = Key::getPubKeyFromPrivKey(secret);

			size_t len = pubKey.GetSize();
			size_t strLen = BRBase58Encode(nullptr, 0, pubKey, len);
			char base58string[strLen];
			BRBase58Encode(base58string, strLen, pubKey, len);

			return base58string;
		}

		CMBlock Key::getPublicKeyByKey(const Key &key) {
			UInt256 secret = key.getSecret();
			if(UInt256IsZero(&secret)) {
				throw std::logic_error("secret key is zero!");
			}
			CMBlock secretMem;
			secretMem.SetMemFixed(secret.u8, sizeof(secret));

			return Key::getPubKeyFromPrivKey(secretMem);
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
			if (UInt256IsZero(&secret)) {
				throw std::logic_error("secret is zero, can't generate publicKey!");
			}
			CMBlock secretData;
			secretData.SetMemFixed(secret.u8, sizeof(secret));

			CMBlock pubKey = Key::getPubKeyFromPrivKey(secretData);

			memset(_key->pubKey, 0, sizeof(_key->pubKey));
			memcpy(_key->pubKey, pubKey, pubKey.GetSize());
		}

		CMBlock Key::compactSign(const CMBlock &data) const {
			CMBlock md32;
			md32.SetMemFixed(data, data.GetSize());

			CMBlock privKey;
			privKey.SetMemFixed(_key->secret.u8, sizeof(_key->secret));

			return BTCKey::SignCompact(privKey, md32);
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
			if(signType == ELA_STANDARD) {
				uInt168.u8[0] = ELA_STAND_ADDRESS;
			} else if (signType == ELA_MULTISIG) {
				uInt168.u8[0] = ELA_MULTISIG_ADDRESS;
			} else if (signType == ELA_CROSSCHAIN) {
				uInt168.u8[0] = ELA_CROSSCHAIN_ADDRESS;
			} else if(signType == ELA_IDCHAIN) {
				uInt168.u8[0] = ELA_IDCHAIN_ADDRESS;
			}
			return uInt168;
		}

		bool Key::verify(const UInt256 &messageDigest, const CMBlock &signature) const {
			CMBlock publicKey = getPubkey();
			std::string pubKey = Utils::encodeHex(publicKey);

			return Key::verifyByPublicKey(pubKey, messageDigest, signature);
		}

		bool Key::isValidBitcoinPrivateKey(const std::string &key) {
			return BRPrivKeyIsValid(key.c_str()) == 1;
		}

		bool Key::isValidBitcoinBIP38Key(const std::string &key) {
			return BRBIP38KeyIsValid(key.c_str()) == 1;
		}

		UInt256 Key::encodeSHA256(const std::string &message) {
			UInt256 md;
			BRSHA256((void *) &md, (void *) message.c_str(), strlen(message.c_str()));
			return md;
		}

		void Key::deriveKeyAndChain(UInt256 &chainCode, const void *seed, size_t seedLen, int depth, ...) {
			va_list ap;

			va_start(ap, depth);
			deriveKeyAndChain(chainCode, seed, seedLen, depth, ap);
			va_end(ap);
		}

		void Key::deriveKeyAndChain(UInt256 &chainCode, const void *seed, size_t seedLen, int depth, va_list vlist) {
			UInt512 I;
			UInt256 secret;

			assert(_key.get() != NULL);
			assert(seed != NULL || seedLen == 0);
			assert(depth >= 0);

			if (_key.get() && (seed || seedLen == 0)) {
				BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
				secret = *(UInt256 *) &I;
				chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
				var_clean(&I);

				for (int i = 0; i < depth; i++) {
					ELABIP32Sequence::CKDpriv(&secret, &chainCode, va_arg(vlist, uint32_t));
				}

				setSecret(secret, true);

				var_clean(&secret, &chainCode);

			}
		}

		const UInt160 Key::hashTo160() {
			UInt160 hash = UINT160_ZERO;
			size_t len = getCompressed() ? 33 : 65;
			//todo add publicKey verify
			if (true) {
				BRHash160(&hash, _key->pubKey, len);
			}
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
			memcpy(&uInt168.u8[1],&hash.u8[0], sizeof(hash.u8) - 1);
			return uInt168;
		}

		void
		Key::calculatePrivateKeyList(BRKey *keys, size_t keysCount, UInt256 *secret, UInt256 *chainCode,
		                             uint32_t chain, const uint32_t *indexes) {
			UInt512 I;

			assert(keys != nullptr || keysCount == 0);
			assert(indexes != nullptr || keysCount == 0);
			if (keys && keysCount > 0 && indexes)
				for (size_t i = 0; i < keysCount; i++) {
					UInt256 code;
					UInt256Set(&code, *chainCode);

					UInt256 privateKey;
					UInt256Set(&privateKey, *secret);

					ELABIP32Sequence::CKDpriv(&privateKey, &code, chain);
					ELABIP32Sequence::CKDpriv(&privateKey, &code, indexes[i]);

					BRKeySetSecret(&keys[i], &privateKey, 1);
				}
		}

		bool Key::verifyByPublicKey(const std::string &publicKey, const UInt256 &messageDigest,
		                            const CMBlock &signature) {
			return BTCKey::VerifyCompact(publicKey, messageDigest, signature);
		}

		std::string Key::keyToRedeemScript(int signType) const {
			if (signType == ELA_MULTISIG_ADDRESS) {
				return "";
			}

			uint64_t size = (getCompressed()) ? 33 : 65;

			ByteStream buff(size + 2);

			buff.put((uint8_t)size);

			buff.putBytes(_key->pubKey, size);

			buff.put((uint8_t)signType);

			uint8_t *script = buff.getBuf();
			size_t scriptLen = 0;
			std::string str = Utils::encodeHexCreate(&scriptLen, script, buff.position());
			delete script;

			return str;
		}

		const UInt256 Key::getSystemAssetId() {
			Transaction elaCoin;
			elaCoin.setTransactionType(ELATransaction::RegisterAsset);
			return elaCoin.getHash();
		}

		CMBlock Key::getPubKeyFromPrivKey(const CMBlock &privKey, int nid) {
			return BTCKey::getPubKeyFromPrivKey(privKey, nid);
		}
	}
}