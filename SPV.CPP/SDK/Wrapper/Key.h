// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEY_H__
#define __ELASTOS_SDK_KEY_H__

#include <boost/shared_ptr.hpp>

#include "BRKey.h"
#include "BRAddress.h"
#include "Wrapper.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace SDK {

		class Key :
				public Wrapper<BRKey> {
		public:
			Key();

			Key(const boost::shared_ptr<BRKey> &brkey);

			Key(const std::string &privKey);

			Key(const CMBlock &privKey);

			Key(const UInt256 &secret, bool compressed);

			Key(const CMBlock &seed, uint32_t chain, uint32_t index);

			virtual std::string toString() const;

			virtual BRKey *getRaw() const;

			UInt256 getSecret() const;

			CMBlock getPubkey() const;

			bool getCompressed() const;

			std::string getPrivKey() const;

			bool setPrivKey(const std::string &privKey);

			CMBlock compactSign(const CMBlock &data) const;

			CMBlock encryptNative(const CMBlock &data, const CMBlock &nonce) const;

			CMBlock decryptNative(const CMBlock &data, const CMBlock &nonce) const;

			std::string address() const;

			CMBlock sign(const UInt256 &messageDigest) const;

			bool verify(const UInt256 &messageDigest, const CMBlock &signature) const;

			std::string keyToAddress(const int signType) const;

			UInt168 keyToUInt168BySignType(const int signType) const;

		public:
			static CMBlock getSeedFromPhrase(const CMBlock &phrase, const std::string &phrasePass = "");

			static CMBlock getAuthPrivKeyForAPI(const CMBlock &seed);

			static std::string getAuthPublicKeyForAPI(const CMBlock &privKey);

			static std::string decryptBip38Key(const std::string &privKey, const std::string &pass);

			static bool isValidBitcoinPrivateKey(const std::string &key);

			static bool isValidBitcoinBIP38Key(const std::string &key);

			static std::string encodeHex(const CMBlock &in);

			static CMBlock decodeHex(const std::string &s);

			static UInt256 encodeSHA256(const std::string &message);

			static void
			deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const void *seed, size_t seedLen, int depth, ...);

			static void
			calculatePrivateKeyList(BRKey keys[], size_t keysCount, UInt256 *secret, UInt256 *chainCode,
			                        uint32_t chain, const uint32_t indexes[]);

			static bool verifyByPublicKey(const std::string &publicKey, const UInt256 &messageDigest,
			                              const CMBlock &signature);

		private:
			bool setSecret(const UInt256 &data, bool compressed);

			static void deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const void *seed, size_t seedLen, int depth,
			                              va_list vlist);

		private:
			boost::shared_ptr<BRKey> _key;
		};

		typedef boost::shared_ptr<Key> KeyPtr;

	}
}

#endif //__ELASTOS_SDK_KEY_H__
