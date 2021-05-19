////////////////////////////////////////////////////////////////////////////////
//
// HDKeychain.h
//
// Copyright (c) 2013-2014 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#ifndef __ELASTOS_SDK_HDKEYCHAIN_H__
#define __ELASTOS_SDK_HDKEYCHAIN_H__

#include <Common/hash.h>
#include <Common/typedefs.h>

#include <stdexcept>

namespace Elastos {
	namespace ElaWallet {

#define BIP32_SEED_KEY "Bitcoin seed"
		const uchar_vector BITCOIN_SEED("426974636f696e2073656564"); // key = "Bitcoin seed"
#define BIP32_HARD                  0x80000000

#define SEQUENCE_GAP_LIMIT_EXTERNAL 10
#define SEQUENCE_GAP_LIMIT_INTERNAL 5
#define SEQUENCE_EXTERNAL_CHAIN     0
#define SEQUENCE_INTERNAL_CHAIN     1

		class HDSeed
		{
			public:
				HDSeed(const bytes_t& seed, const bytes_t& coin_seed = BITCOIN_SEED)
				{
					bytes_t hmac = hmac_sha512(coin_seed, seed);
					master_key_.assign(hmac.begin(), hmac.begin() + 32);
					master_chain_code_.assign(hmac.begin() + 32, hmac.end());
				}

				~HDSeed() {
					seed_.clean();
					master_key_.clean();
					master_chain_code_.clean();
				}

				const bytes_t& getSeed() const { return seed_; }
				const bytes_t& getMasterKey() const { return master_key_; }
				const bytes_t& getMasterChainCode() const { return master_chain_code_; }
				bytes_t getExtendedKey(bool bPrivate = false) const;

			private:
				bytes_t seed_;
				bytes_t master_key_;
				bytes_t master_chain_code_;
		};

		class HDKeychain
		{
			public:
				HDKeychain() { _valid = false; }
				HDKeychain(const bytes_t& key, const bytes_t& chain_code, uint32_t child_num = 0, uint32_t parent_fp = 0, uint32_t depth = 0);
				HDKeychain(const bytes_t& extkey);
				HDKeychain(const HDKeychain& source);

				~HDKeychain() { _key.clean(); _chain_code.clean(); }

				HDKeychain& operator=(const HDKeychain& rhs);

				explicit operator bool() const { return _valid; }


				bool operator==(const HDKeychain& rhs) const;
				bool operator!=(const HDKeychain& rhs) const;

				// Serialization
				bytes_t extkey() const;

				// Accessor Methods
				uint32_t version() const { return _version; }
				int depth() const { return _depth; }
				uint32_t parent_fp() const { return _parent_fp; }
				uint32_t child_num() const { return _child_num; }
				const bytes_t& chain_code() const { return _chain_code; }
				const bytes_t& key() const { return _key; }

				bytes_t privkey() const;
				const bytes_t& pubkey() const { return _pubkey; }
				bytes_t uncompressed_pubkey() const;

				bool isPrivate() const { return (_key.size() == 33 && _key[0] == 0x00); }
				bytes_t hash() const; // hash is ripemd160(sha256(pubkey))
				uint32_t fp() const; // fingerprint is first 32 bits of hash
				bytes_t full_hash() const; // full_hash is ripemd160(sha256(pubkey + chain_code))

				bool valid() const { return _valid; }

				HDKeychain getPublic() const;
				HDKeychain getChild(uint32_t i) const;
				HDKeychain getChild(const std::string& path) const;
				HDKeychain getChildNode(uint32_t i, bool private_derivation = false) const {
					uint32_t mask = private_derivation ? 0x80000000ull : 0x00000000ull;
					return getChild(mask).getChild(i);
				}

				// Precondition: i >= 1
				bytes_t getPrivateSigningKey(uint32_t i) const {
					//if (i == 0) throw std::runtime_error("Signing key index cannot be zero.");
					return getChild(i).privkey();
				}

				// Precondition: i >= 1
				bytes_t getPublicSigningKey(uint32_t i, bool bCompressed = true) const {
					//if (i == 0) throw std::runtime_error("Signing key index cannot be zero.");
					return bCompressed ? getChild(i).pubkey() : getChild(i).uncompressed_pubkey();
				}

				static void setVersions(uint32_t priv_version, uint32_t pub_version) {
					_priv_version = priv_version; _pub_version = pub_version;
				}

				std::string toString() const;

			private:
				static uint32_t _priv_version;
				static uint32_t _pub_version;

				uint32_t _version;
				unsigned char _depth;
				uint32_t _parent_fp;
				uint32_t _child_num;
				bytes_t _chain_code; // 32 bytes
				bytes_t _key;        // 33 bytes, first byte is 0x00 for private key

				bytes_t _pubkey;

				bool _valid;

				void updatePubkey();
		};

		typedef boost::shared_ptr<HDKeychain> HDKeychainPtr;
		typedef std::vector<HDKeychainPtr> HDKeychainArray;

	}
}

#endif // __ELASTOS_SDK_HDKEYCHAIN_H__
