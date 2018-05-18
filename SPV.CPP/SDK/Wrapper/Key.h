// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEY_H__
#define __ELASTOS_SDK_KEY_H__

#include <boost/shared_ptr.hpp>

#include "BRKey.h"

#include "Wrapper.h"
#include "c_util.h"

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

		public:
			static CMBlock getSeedFromPhrase(const CMBlock &phrase);

			static CMBlock getAuthPrivKeyForAPI(const CMBlock &seed);

			static std::string getAuthPublicKeyForAPI(const CMBlock &privKey);

			static std::string decryptBip38Key(const std::string &privKey, const std::string &pass);

			static bool isValidBitcoinPrivateKey(const std::string &key);

			static bool isValidBitcoinBIP38Key(const std::string &key);

			static std::string encodeHex(const CMBlock &in);

			static CMBlock decodeHex(const std::string &s);

			static UInt256 encodeSHA256(const std::string &message);

		private:
			bool setSecret(const UInt256 &data, bool compressed);

		private:
			boost::shared_ptr<BRKey> _key;
		};

		typedef boost::shared_ptr<Key> KeyPtr;

	}
}

#endif //__ELASTOS_SDK_KEY_H__
