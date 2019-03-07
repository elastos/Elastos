// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEY_H__
#define __ELASTOS_SDK_KEY_H__

#include <SDK/Common/CMemBlock.h>
#include <SDK/Common/BigNum.h>

#include <Core/BRInt.h>

#include <boost/shared_ptr.hpp>
#include <openssl/obj_mac.h>
#include <openssl/ec.h>

namespace Elastos {
	namespace ElaWallet {

#define BITCOIN_PRIVKEY      128
#define BITCOIN_PRIVKEY_TEST 239

		enum SignType {
			SignTypeInvalid    = 0,
			SignTypeStandard   = 0xAC,
			SignTypeMultiSign  = 0xAE,
			SignTypeCrossChain = 0xAF,
			SignTypeIDChain    = 0xAD,
			SignTypeDestroy    = 0xAA,
		};

		enum Prefix {
			PrefixStandard   = 0x21,
			PrefixMultiSign  = 0x12,
			PrefixCrossChain = 0x4B,
			PrefixDeposit    = 0x1F,
			PrefixIDChain    = 0x67,
			PrefixDestroy    = 0,
		};

		struct ECPoint {
			uint8_t p[33];
		};

		class Key {
		public:
			Key();

			Key(const Key &key);

			Key(const UInt256 &secret, bool compressed);

			~Key();

			Key &operator=(const Key &key);

			bool SetPubKey(const CMBlock &pubKey);

			std::string PrivKey() const;

			CMBlock PubKey();

			const UInt256 &GetSecret() const;

			bool SetSecret(const UInt256 &secret, bool compressed);

			bool PrivKeyIsValid(const std::string &privKey) const;

			bool SetPrivKey(const std::string &privKey);

			SignType PrefixToSignType(Prefix prefix) const;

			CMBlock MultiSignRedeemScript(uint8_t m, const std::vector<CMBlock> &pubKeys);

			CMBlock RedeemScript(Prefix prefix) const;

			static UInt168 CodeToProgramHash(Prefix prefix, const CMBlock &code);

			UInt168 ProgramHash(Prefix prefix);

			std::string GetAddress(Prefix prefix) const;

			CMBlock Sign(const UInt256 &md) const;

			CMBlock Sign(const std::string &message) const;

			CMBlock Sign(const CMBlock &message) const;

			bool Verify(const std::string &message, const CMBlock &signature) const;

			bool Verify(const UInt256 &md, const CMBlock &signature) const;

			bool Valid() const;

			static bool PubKeyIsValid(const void *pubKey, size_t len);

		private:
			bool PubKeyEmpty() const;

			void GeneratePubKey() const;

			void Clean();

			bool Compare(const CMBlock &a, const CMBlock &b) const;

			BigNum PubKeyDecodePointX(const CMBlock &pubKey) const;

		private:
			UInt256 _secret;
			mutable uint8_t _pubKey[65];
			mutable bool _compressed;
		};

		typedef boost::shared_ptr<Key> KeyPtr;

	}
}

#endif //__ELASTOS_SDK_KEY_H__
