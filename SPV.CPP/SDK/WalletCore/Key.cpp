// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Key.h"

#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/secp256k1_openssl.h>

#include <cstring>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <openssl/ecdsa.h>

namespace Elastos {
	namespace ElaWallet {

		Key::Key() {
		}

		Key::Key(const bytes_t &key) {
			if (key.size() == 32) {
				_key.setPrivKey(key);
			} else if (key.size() == 33) {
				_key.setPubKey(key);
			} else {
				ErrorChecker::ThrowLogicException(Error::Key, "invalid key");
			}
		}

		Key::Key(const HDKeychain &keychain) {
			operator=(keychain);
		}

		Key::Key(const Key &key) {
			operator=(key);
		}

		Key::~Key() {
		}

		Key &Key::operator=(const HDKeychain &keychain) {
			ErrorChecker::CheckLogic(!keychain.valid(), Error::Key, "keychain is not valid");

			if (keychain.isPrivate()) {
				_key.setPrivKey(keychain.privkey());
			} else {
				_key.setPubKey(keychain.pubkey());
			}
			return *this;
		}

		Key &Key::operator=(const Key &key) {
			_key = key._key;
			return *this;
		}

		bool Key::SetPubKey(const bytes_t &pubKey) {
			return nullptr != _key.setPubKey(pubKey);
		}

		bytes_t Key::PubKey(bool compress) const {
			return _key.getPubKey(compress);
		}

		bytes_t Key::PrvKey() const {
			return _key.getPrivKey();
		}

		bool Key::SetPrvKey(const bytes_t &prv) {
			return nullptr != _key.setPrivKey(prv);
		}


		bytes_t Key::Sign(const std::string &message) const {
			return Sign(bytes_t(message.c_str(), message.size()));
		}

		bytes_t Key::Sign(const bytes_t &message) const {
			uint256 digest(sha256(message));
			return Sign(digest);
		}

		bytes_t Key::Sign(const uint256 &digest) const {
			bytes_t signature;
			bool success = false;

			ErrorChecker::CheckLogic(_key.getKey() == nullptr, Error::Sign, "invalid key for signing");
//			ErrorChecker::CheckLogic(!EC_KEY_can_sign(_key.getKey()), Error::Sign, "key can't use for signing");

			ECDSA_SIG *sig = ECDSA_do_sign(digest.begin(), digest.size(), _key.getKey());
			if (sig != nullptr) {
				const BIGNUM *r = nullptr;
				const BIGNUM *s = nullptr;
				ECDSA_SIG_get0(sig, &r, &s);
				if (BN_num_bits(r) <= 256 && BN_num_bits(s) <= 256) {
					success = true;
					bytes_t arrBin(32);
					signature.resize(64, 0);

					int len = BN_bn2bin(r, &arrBin[0]);
					memcpy(&signature[32 - len], &arrBin[0], len);

					len = BN_bn2bin(s, &arrBin[0]);
					memcpy(&signature[32 + 32 - len], &arrBin[0], len);
				}
				ECDSA_SIG_free(sig);
			}

			if (!success)
				ErrorChecker::ThrowLogicException(Error::Sign, "Sign fail");

			return signature;
		}

		bool Key::Verify(const std::string &message, const bytes_t &signature) const {
			return Verify(bytes_t(message.c_str(), message.size()), signature);
		}

		bool Key::Verify(const bytes_t &message, const bytes_t &signature) const {
			uint256 digest(sha256(message));
			return Verify(digest, signature);
		}

		bool Key::Verify(const uint256 &digest, const bytes_t &signature) const {
			bool result = false;

			ErrorChecker::CheckLogic(_key.getKey() == nullptr, Error::Sign, "invalid key for verify");

			ECDSA_SIG *sig = ECDSA_SIG_new();
			if (nullptr != sig) {
				BIGNUM *r = BN_bin2bn(&signature[0], 32, nullptr);
				BIGNUM *s = BN_bin2bn(&signature[32], 32, nullptr);
				ECDSA_SIG_set0(sig, r, s);
				if (1 == ECDSA_do_verify(digest.begin(), digest.size(), sig, _key.getKey())) {
					result = true;
				}
				ECDSA_SIG_free(sig);
			}

			return result;
		}

	}
}
