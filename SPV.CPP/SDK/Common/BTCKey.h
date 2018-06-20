// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BTCKEY_H__
#define __ELASTOS_SDK_BTCKEY_H__

#include <openssl/obj_mac.h>
#include <openssl/ec.h>
#include <BRInt.h>

#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {
		class BTCKey {
		public:
			static bool generateKey(CMemBlock<uint8_t> &privKey, CMemBlock<uint8_t> &pubKey, int nid = NID_secp256k1);

			static CMemBlock<uint8_t> getPubKeyFromPrivKey(CMemBlock<uint8_t> privKey, int nid = NID_secp256k1);

			static bool PublickeyIsValid(CMemBlock<uint8_t> pubKey, int nid = NID_secp256k1);

			static CMBlock SignCompact(const CMemBlock<uint8_t> &privKey, const CMemBlock<uint8_t> msg);

			static bool VerifyCompact(const std::string &publicKey, const UInt256 &msg, const CMBlock &signature);

			static size_t ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const uint8_t *msg, size_t msglen,
			                                        size_t recid, size_t check);
		};
	}
}


#endif //__ELASTOS_SDK_BTCKEY_H__
