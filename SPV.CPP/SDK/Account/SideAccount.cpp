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
#include "SideAccount.h"
#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {
		SideAccount::SideAccount(const uint256 &genesis_hash)
			: parent(AccountPtr(new FakeParent())) {
			// Create side chain redeem script according to the genesis hash.
			bytes_t data;
			data.push_back(genesis_hash.size());
			for (auto b : genesis_hash.bytes()) {
				data.push_back(b);
			}
			data.push_back(0xAF); // The cross chain op code.

            side_address = AddressPtr(new Address());
            side_address->SetRedeemScript(PrefixCrossChain, data);
		}

		SideAccount::~SideAccount() {
		}

		nlohmann::json SideAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = Parent()->GetBasicInfo();
			j["CoinIndex"] = 0;
			return j;
		}

		void SideAccount::Init() {}

		void SideAccount::InitCID() {}

		bool SideAccount::IsSingleAddress() const { return true; }

		bool SideAccount::IsProducerDepositAddress(const AddressPtr &address) const { return false; }

		bool SideAccount::IsOwnerAddress(const AddressPtr &address) const { return false; }

		bool SideAccount::IsCRDepositAddress(const AddressPtr &address) const { return false; }

		void SideAccount::SetUsedAddresses(const AddressSet &addresses) { }

		bool SideAccount::AddUsedAddress(const AddressPtr &) { return false; }

		size_t SideAccount::GetAllAddresses(AddressArray &addr, uint32_t, size_t, bool) const {
			addr.clear();
			addr.push_back(side_address);
			return addr.size();
		}

		size_t SideAccount::GetAllCID(AddressArray &did, uint32_t start, size_t count) const {
			return 0;
		}

		size_t SideAccount::GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
		                        bool containInternal) const {
			return 0;
		}

		AddressArray SideAccount::UnusedAddresses(uint32_t, bool) {
			return {side_address};
		}

		bytes_t SideAccount::OwnerPubKey() const { return bytes_t(); }

		bytes_t SideAccount::DIDPubKey() const { return bytes_t(); }

		void SideAccount::SignTransaction(const TransactionPtr &, const std::string &) const {}

		Key SideAccount::GetKeyWithDID(const AddressPtr &did, const std::string &payPasswd) const {
			return Key();
		}

		Key SideAccount::DeriveOwnerKey(const std::string &) { return Key(); }

		Key SideAccount::DeriveDIDKey(const std::string &payPasswd) { return Key(); }

		bool SideAccount::ContainsAddress(const AddressPtr &address) const { return *side_address == *address; }

		bool SideAccount::GetCodeAndPath(const AddressPtr &, bytes_t &, std::string &) const { return false; }

		size_t SideAccount::InternalChainIndex(const TransactionPtr &tx) const { return -1; }

		size_t SideAccount::ExternalChainIndex(const TransactionPtr &tx) const { return -1; }

		AccountPtr SideAccount::Parent() const { return parent; }
	} // namespace ElaWallet
} // namespace Elastos
