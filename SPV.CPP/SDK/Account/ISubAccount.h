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
#ifndef __ELASTOS_SDK_ISUBACCOUNT_H__
#define __ELASTOS_SDK_ISUBACCOUNT_H__

#include <Common/Lockable.h>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;

		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class ISubAccount {
		public:
			virtual ~ISubAccount() {}

			virtual nlohmann::json GetBasicInfo() const = 0;

			virtual void Init() = 0;

			virtual void InitCID() = 0;

			virtual bool IsSingleAddress() const = 0;

			virtual bool IsProducerDepositAddress(const AddressPtr &address) const = 0;

			virtual bool IsOwnerAddress(const AddressPtr &address) const = 0;

			virtual bool IsCRDepositAddress(const AddressPtr &address) const = 0;

			virtual void SetUsedAddresses(const AddressSet &addresses) = 0;

			virtual bool AddUsedAddress(const AddressPtr &address) = 0;

			virtual size_t GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const = 0;

			virtual size_t GetAllCID(AddressArray &did, uint32_t start, size_t count) const = 0;

			virtual size_t GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
			                                bool containInternal) const = 0;

			virtual AddressArray UnusedAddresses(uint32_t gapLimit, bool internal) = 0;

			virtual bool ContainsAddress(const AddressPtr &address) const = 0;

			virtual bytes_t OwnerPubKey() const = 0;

			virtual bytes_t DIDPubKey() const = 0;

			virtual void SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) const = 0;

			virtual Key GetKeyWithDID(const AddressPtr &did, const std::string &payPasswd) const = 0;

			virtual Key DeriveOwnerKey(const std::string &payPasswd) = 0;

			virtual Key DeriveDIDKey(const std::string &payPasswd) = 0;

			virtual bool GetCodeAndPath(const AddressPtr &addr, bytes_t &code, std::string &path) const = 0;

			virtual size_t InternalChainIndex(const TransactionPtr &tx) const = 0;

			virtual size_t ExternalChainIndex(const TransactionPtr &tx) const = 0;

			virtual AccountPtr Parent() const = 0;
		};

		typedef boost::shared_ptr<ISubAccount> SubAccountPtr;

	}
}

#endif //__ELASTOS_SDK_ISUBACCOUNT_H__
