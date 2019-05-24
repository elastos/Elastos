// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_STANDARDSINGLESUBACCOUNT_H__
#define __ELASTOS_SDK_STANDARDSINGLESUBACCOUNT_H__

#include "SingleSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class StandardSingleSubAccount : public SingleSubAccount {
		public:
			StandardSingleSubAccount(const HDKeychain &masterPubKey, const bytes_t &ownerPubKey,
									 IAccount *account, uint32_t coinIndex);

			virtual bytes_t GetRedeemScript(const Address &addr) const;

			virtual bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd);

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal);

			virtual size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) const;

			virtual bool ContainsAddress(const Address &address) const;

			virtual Key DeriveOwnerKey(const std::string &payPasswd);

			virtual size_t TxInternalChainIndex(const TransactionPtr &tx) const;

			virtual size_t TxExternalChainIndex(const TransactionPtr &tx) const;

		private:
			Address _address;
		};

	}
}

#endif //__ELASTOS_SDK_STANDARDSINGLESUBACCOUNT_H__
