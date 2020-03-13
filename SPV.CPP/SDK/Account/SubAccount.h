/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ELASTOS_SDK_SUBACCOUNT_H__
#define __ELASTOS_SDK_SUBACCOUNT_H__

#include "Account.h"
#include "ISubAccount.h"

#include <Common/Lockable.h>

#include <set>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class SubAccount : public ISubAccount {
		public:
			SubAccount(const AccountPtr &parent, uint32_t coinIndex);

			~SubAccount();

			nlohmann::json GetBasicInfo() const;

			void Init();

			void InitCID();

			bool IsSingleAddress() const;

			bool IsProducerDepositAddress(const AddressPtr &address) const;

			bool IsOwnerAddress(const AddressPtr &address) const;

			bool IsCRDepositAddress(const AddressPtr &address) const;

			void SetUsedAddresses(const AddressSet &addresses);

			bool AddUsedAddress(const AddressPtr &address);

			size_t GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const;

			size_t GetAllCID(AddressArray &did, uint32_t start, size_t count) const;

			AddressArray UnusedAddresses(uint32_t gapLimit, bool internal);

			bool ContainsAddress(const AddressPtr &address) const;

			size_t GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
			                        bool containInternal) const;

			bytes_t OwnerPubKey() const;

			bytes_t DIDPubKey() const;

			void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) const;

			Key GetKeyWithDID(const AddressPtr &did, const std::string &payPasswd) const;

			Key DeriveOwnerKey(const std::string &payPasswd);

			Key DeriveDIDKey(const std::string &payPasswd);

			bool GetCodeAndPath(const AddressPtr &addr, bytes_t &code, std::string &path) const;

			size_t InternalChainIndex(const TransactionPtr &tx) const;

			size_t ExternalChainIndex(const TransactionPtr &tx) const;

			AccountPtr Parent() const;
		private:
			uint32_t _coinIndex;
			AddressArray _internalChain, _externalChain, _cid;
			AddressSet _usedAddrs, _allAddrs, _allCID;
			mutable AddressPtr _depositAddress, _ownerAddress, _crDepositAddress;

			AccountPtr _parent;
		};

	}
}

#endif //__ELASTOS_SDK_SUBACCOUNT_H__
