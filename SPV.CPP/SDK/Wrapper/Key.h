// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEY_H__
#define __ELASTOS_SDK_KEY_H__

#include <boost/shared_ptr.hpp>

#include "BRKey.h"

#include "Wrapper.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class Key :
			public Wrapper<BRKey> {
		public:
			Key();

			Key(std::string privKey);

			Key(ByteData privKey);

			Key(UInt256 secret, bool compressed);

			Key(ByteData seed, uint32_t chain, uint32_t index);

			virtual std::string toString() const;

			virtual BRKey *getRaw() const;

			UInt256 getSecret() const;

			ByteData getPubkey() const;

			bool getCompressed() const;

			std::string getPrivKey() const;

			bool setPrivKey(std::string privKey);

			ByteData compactSign(ByteData data) const;

			ByteData encryptNative(ByteData data, ByteData nonce) const;

			ByteData decryptNative(ByteData data, ByteData nonce) const;

			std::string address() const;

			ByteData sign(UInt256 messageDigest) const;

			bool verify(UInt256 messageDigest, ByteData signature) const;

		public:
			static ByteData getSeedFromPhrase(ByteData phrase);

			static ByteData getAuthPrivKeyForAPI(ByteData seed);

			static std::string getAuthPublicKeyForAPI(ByteData privKey);

			static std::string decryptBip38Key(std::string privKey, std::string pass);

			static bool isValidBitcoinPrivateKey(std::string key);

			static bool isValidBitcoinBIP38Key(std::string key);

			static std::string encodeHex(ByteData in);

			static ByteData decodeHex(std::string s);

			static UInt256 encodeSHA256(std::string message);

		private:
			bool setSecret(UInt256 data, bool compressed);

		private:
			boost::shared_ptr<BRKey> _key;
		};

		typedef boost::shared_ptr<Key> KeyPtr;

	}
}

#endif //__ELASTOS_SDK_KEY_H__
