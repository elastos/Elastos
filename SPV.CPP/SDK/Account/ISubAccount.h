// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBACCOUNT_H__
#define __ELASTOS_SDK_ISUBACCOUNT_H__

#include "IAccount.h"

#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Lockable;
		class Wallet;

		namespace BIP32 {
			enum Account {
				Default,
				Vote
			};

			enum Change {
				External,
				Internal
			};
		}

		class ISubAccount {
		public:
			virtual ~ISubAccount() {}

			virtual nlohmann::json GetBasicInfo() const = 0;

			virtual bool IsSingleAddress() const = 0;

			virtual IAccount *GetParent() const = 0;

			virtual void InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) = 0;

			virtual bytes_t GetRedeemScript(const Address &addr) const = 0;

			virtual bool FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd) = 0;

			virtual bytes_t GetMultiSignPublicKey() const = 0;

			virtual Key DeriveMultiSignKey(const std::string &payPassword) = 0;

			virtual void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) = 0;

			virtual void AddUsedAddrs(const TransactionPtr &tx) = 0;

			virtual std::vector<Address> UnusedAddresses(uint32_t gapLimit, bool internal) = 0;

			virtual size_t GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) const = 0;

			virtual bool ContainsAddress(const Address &address) const = 0;

			virtual bool IsDepositAddress(const Address &address) const = 0;

			virtual bool IsAddressUsed(const Address &address) const = 0;

			virtual void ClearUsedAddresses() = 0;

			virtual Key DeriveVoteKey(const std::string &payPasswd) = 0;

			virtual bytes_t GetVotePublicKey() const = 0;

		};

		typedef boost::shared_ptr<ISubAccount> SubAccountPtr;

	}
}

#endif //__ELASTOS_SDK_ISUBACCOUNT_H__
