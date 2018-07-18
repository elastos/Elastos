// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPRIVKEY_H__
#define __ELASTOS_SDK_MASTERPRIVKEY_H__

#include "Key.h"
#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class MasterPrivKey {
		public:
			MasterPrivKey();

			~MasterPrivKey();

			const CMBlock &GetEncryptedKey() const;

			void SetEncryptedKey(const CMBlock &data);

			const UInt256 &GetChainCode() const;

			void SetChainCode(const UInt256 &chainCode);

		private:
			CMBlock _encryptedKey;
			UInt256 _chainCode;
		};

	}
}

#endif //__ELASTOS_SDK_MASTERPRIVKEY_H__
