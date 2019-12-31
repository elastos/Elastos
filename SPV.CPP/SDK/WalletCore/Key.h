// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEY_H__
#define __ELASTOS_SDK_KEY_H__

#include <Common/uint256.h>
#include <Common/typedefs.h>
#include <WalletCore/secp256k1_openssl.h>
#include <WalletCore/HDKeychain.h>

#include <boost/shared_ptr.hpp>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>

namespace Elastos {
	namespace ElaWallet {

		class Key {
		public:
			Key();

			Key(const bytes_t &key);

			Key(const HDKeychain &keychain);

			Key(const Key &key);

			~Key();

			Key &operator=(const HDKeychain &keychain);

			Key &operator=(const Key &key);

			bool SetPubKey(const bytes_t &pub);

			bytes_t PubKey(bool compress = true) const;

			bytes_t PrvKey() const;

			bool SetPrvKey(const bytes_t &prv);

			bytes_t Sign(const std::string &message) const;

			bytes_t Sign(const bytes_t &message) const;

			bytes_t Sign(const uint256 &digest) const;

			bool Verify(const std::string &message, const bytes_t &signature) const;

			bool Verify(const bytes_t &message, const bytes_t &signature) const;

			bool Verify(const uint256 &digest, const bytes_t &signature) const;

		private:
			secp256k1_key _key;
		};

		typedef boost::shared_ptr<Key> KeyPtr;

	}
}

#endif //__ELASTOS_SDK_KEY_H__
