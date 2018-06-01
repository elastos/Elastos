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
#include "BRKey.h"

#include "Common/Utils.h"
#include "Log.h"
#include "Key.h"

#define BIP32_SEED_KEY "ELA seed"

namespace Elastos {
	namespace SDK {
		namespace {
			// Private parent key -> private child key
			//
			// CKDpriv((kpar, cpar), i) -> (ki, ci) computes a child extended private key from the parent extended private key:
			//
			// - Check whether i >= 2^31 (whether the child is a hardened key).
			//     - If so (hardened child): let I = HMAC-SHA512(Key = cpar, Data = 0x00 || ser256(kpar) || ser32(i)).
			//       (Note: The 0x00 pads the private key to make it 33 bytes long.)
			//     - If not (normal child): let I = HMAC-SHA512(Key = cpar, Data = serP(point(kpar)) || ser32(i)).
			// - Split I into two 32-byte sequences, IL and IR.
			// - The returned child key ki is parse256(IL) + kpar (mod n).
			// - The returned chain code ci is IR.
			// - In case parse256(IL) >= n or ki = 0, the resulting key is invalid, and one should proceed with the next value for i
			//   (Note: this has probability lower than 1 in 2^127.)
			//
			static void _CKDpriv(UInt256 *k, UInt256 *c, uint32_t i) {
				uint8_t buf[sizeof(BRECPoint) + sizeof(i)];
				UInt512 I;

				if (i & BIP32_HARD) {
					buf[0] = 0;
					UInt256Set(&buf[1], *k);
				} else BRSecp256k1PointGen((BRECPoint *) buf, k);

				UInt32SetBE(&buf[sizeof(BRECPoint)], i);

				BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf,
				       sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)

				BRSecp256k1ModAdd(k, (UInt256 *) &I); // k = IL + k (mod n)
				*c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR

				var_clean(&I);
				mem_clean(buf, sizeof(buf));
			}
		}

		Key::Key() {
			_key = boost::shared_ptr<BRKey>(new BRKey);
			memset(_key.get(), 0, sizeof(BRKey));
		}

		Key::Key(BRKey *brkey) {
			_key = boost::shared_ptr<BRKey>(brkey);
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
			return Utils::UInt256ToString(_key->secret);
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
			BRBIP32APIAuthKey(&key, (void *) seed, seed.GetSize());
			char rawKey[BRKeyPrivKey(&key, nullptr, 0)];
			BRKeyPrivKey(&key, rawKey, sizeof(rawKey));

			CMBlock ret(sizeof(rawKey));
			memcpy(ret, &rawKey, sizeof(rawKey));

			return ret;
		}

		std::string Key::getAuthPublicKeyForAPI(const CMBlock &privKey) {
			BRKey key;
			BRKeySetPrivKey(&key, (const char *) (void *) privKey);
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
			CMBlock out(1024);
			size_t outSize = BRChacha20Poly1305AEADDecrypt(out, data.GetSize(), _key.get(), nonce,
			                                               data, data.GetSize(), nullptr, 0);
			out.Resize(outSize);
			return out;
		}


		std::string Key::address() const {
			return keyToAddress(ELA_STANDARD);
		}

		CMBlock Key::sign(const UInt256 &messageDigest) const {
			CMBlock signature(256);
			size_t signatureLen = BRKeySign(_key.get(), signature, 256, messageDigest);
			assert (signatureLen <= 256);
			signature.Resize(signatureLen);
			return signature;
		}

		std::string Key::keyToAddress(const int signType) const {
			UInt168 hash;
			uint8_t data[21];
			BRAddress address = BR_ADDRESS_NONE;
			size_t addrLen = sizeof(address);

			hash = keyToUInt168BySignType(signType);

			UInt168Set(&data[0], hash);
			if (! UInt168IsZero(&hash)) {
				BRBase58CheckEncode(address.s, addrLen, data, sizeof(data));
			}

			assert(address.s[0] != '\0');

			return address.s;
		}

		UInt168 Key::keyToUInt168BySignType(const int signType) const {
			UInt168 uInt168 = BRKeyHash168(_key.get());
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

		void Key::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const void *seed, size_t seedLen, int depth, ...) {
			va_list ap;

			va_start(ap, depth);
			deriveKeyAndChain(key, chainCode, seed, seedLen, depth, ap);
			va_end(ap);
		}

		void Key::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const void *seed, size_t seedLen, int depth,
		                            va_list vlist) {
			UInt512 I;
			UInt256 secret;

			assert(key != NULL);
			assert(seed != NULL || seedLen == 0);
			assert(depth >= 0);

			if (key && (seed || seedLen == 0)) {
				BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
				secret = *(UInt256 *) &I;
				chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
				var_clean(&I);

				for (int i = 0; i < depth; i++) {
					_CKDpriv(&secret, &chainCode, va_arg(vlist, uint32_t));
				}

				BRKeySetSecret(key, &secret, 1);
				var_clean(&secret, &chainCode);
			}
		}

		void
		Key::calculatePrivateKeyList(BRKey *keys, size_t keysCount, UInt256 *secret, UInt256 *chainCode,
		                             uint32_t chain, const uint32_t *indexes) {
			UInt512 I;
			UInt256 *s, *c;

			assert(keys != nullptr || keysCount == 0);
			assert(indexes != nullptr || keysCount == 0);

			if (keys && keysCount > 0 && indexes) {

				_CKDpriv(secret, chainCode, 0 | BIP32_HARD); // path m/0H
				_CKDpriv(secret, chainCode, chain); // path m/0H/chain

				for (size_t i = 0; i < keysCount; i++) {
					s = secret;
					c = chainCode;
					_CKDpriv(s, c, indexes[i]); // index'th key in chain
					BRKeySetSecret(&keys[i], s, 1);
				}
			}
		}

		bool Key::verifyByPublicKey(const std::string &publicKey, const UInt256 &messageDigest,
		                            const CMBlock &signature) {
			size_t len = publicKey.size();
			int r = 0;
			secp256k1_context *_ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);;
			secp256k1_pubkey pk;
			secp256k1_ecdsa_signature s;
			uint8_t pubKey[65];
			memset(pubKey, 0, sizeof(pubKey));
			memcpy(pubKey, publicKey.c_str(), len);

			if (len > 0 && secp256k1_ec_pubkey_parse(_ctx, &pk, pubKey, len) &&
			    secp256k1_ecdsa_signature_parse_der(_ctx, &s, signature, signature.GetSize())) {
				if (secp256k1_ecdsa_verify(_ctx, &s, messageDigest.u8, &pk) == 1) r = 1; // success is 1, all other values are fail
			}
			return r;
		}

		std::string Key::keyToRedeemScript(int signType) const {
			//todo complete me
			return "";
		}


	}
}