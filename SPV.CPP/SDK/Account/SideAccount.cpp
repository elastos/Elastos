// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SideAccount.h"
#include <SDK/Common/uint256.h>

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

			side_address.SetRedeemScript(PrefixCrossChain, data);
		}

		SideAccount::~SideAccount() {
		}

		nlohmann::json SideAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = Parent()->GetBasicInfo();
			j["CoinIndex"] = 0;
			return j;
		}

		void SideAccount::Init(const std::vector<TransactionPtr> &) {}

		void SideAccount::InitDID() {}

		bool SideAccount::IsSingleAddress() const { return true; }

		bool SideAccount::IsProducerDepositAddress(const Address &address) const { return false; }

		bool SideAccount::IsOwnerAddress(const Address &address) const { return false; }

		bool SideAccount::IsCRDepositAddress(const Address &address) const { return false; }

		void SideAccount::AddUsedAddrs(const Address &) {}

		size_t SideAccount::GetAllAddresses(std::vector<Address> &addr, uint32_t, size_t, bool) const {
			addr.clear();
			addr.push_back(side_address);
			return addr.size();
		}

		size_t SideAccount::GetAllDID(std::vector<Address> &did, uint32_t start, size_t count) const {
			return 0;
		}

		size_t SideAccount::GetAllPublickeys(std::vector<std::string> &pubkeys, uint32_t start, size_t count,
		                        bool containInternal) const {
			return 0;
		}

		std::vector<Address> SideAccount::UnusedAddresses(uint32_t, bool) {
			std::vector<Address> addrs;
			addrs.push_back(side_address);
			return addrs;
		}

		bytes_t SideAccount::OwnerPubKey() const { return bytes_t(); }

		bytes_t SideAccount::DIDPubKey() const { return bytes_t(); }

		void SideAccount::SignTransaction(const TransactionPtr &, const std::string &) {}

		std::string SideAccount::SignWithDID(const Address &did, const std::string &msg, const std::string &payPasswd) {
			return "";
		}

		Key SideAccount::DeriveOwnerKey(const std::string &) { return Key(); }

		Key SideAccount::DeriveDIDKey(const std::string &payPasswd) { return Key(); }

		bool SideAccount::ContainsAddress(const Address &address) const { return side_address == address; }

		void SideAccount::ClearUsedAddresses() {}

		bool SideAccount::GetCodeAndPath(const Address &, bytes_t &, std::string &) const { return false; }

		size_t SideAccount::InternalChainIndex(const TransactionPtr &tx) const { return -1; }

		size_t SideAccount::ExternalChainIndex(const TransactionPtr &tx) const { return -1; }

		AccountPtr SideAccount::Parent() const { return parent; }
	} // namespace ElaWallet
} // namespace Elastos
