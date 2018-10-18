// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cmake-build-debug/ThirdParty/boost/install/include/boost/function.hpp>
#include "GroupedTransactions.h"

namespace Elastos {
	namespace ElaWallet {

		const std::vector<TransactionPtr> &GroupedTransactions::GetTransactions(uint32_t assetTableID) const {
			return _groupedTransactions[assetTableID];
		}

		const std::vector<TransactionPtr> &GroupedTransactions::GetAllTransactions() const {
			return _transactions;
		}

		void GroupedTransactions::SortTransaction() {
			std::sort(_transactions.begin(), _transactions.end(),
					  [](const TransactionPtr &first, const TransactionPtr &second) {
						  return first->getTimestamp() < second->getTimestamp();
					  });

			_groupedTransactions.clear();
			for (size_t i = 0; i < _transactions.size(); ++i) {
				if (_groupedTransactions.find(_transactions[i]->GetAssetTableID()) == _groupedTransactions.end())
					_groupedTransactions[_transactions[i]->GetAssetTableID()] = {_transactions[i]};
				else
					_groupedTransactions[_transactions[i]->GetAssetTableID()].push_back(_transactions[i]);
			}
		}

		void GroupedTransactions::Append(const TransactionPtr &transaction) {
			transaction->SetAssetTableID(FindAssetTableIDByAssetID(transaction->getHash()));
			_transactions.push_back(transaction);
		}

		TransactionPtr &GroupedTransactions::operator[](size_t index) {
			return _transactions[index];
		}

		size_t GroupedTransactions::GetSize() const {
			return _transactions.size();
		}

		void GroupedTransactions::RemoveAt(size_t index) {
			if (index >= _transactions.size()) return;

			std::vector<TransactionPtr> &txs = _groupedTransactions[_transactions[index]->GetAssetTableID()];
			txs.erase(std::find(txs.begin(), txs.end(), _transactions[index]));
			_transactions.erase(_transactions.begin() + index);
		}

		bool GroupedTransactions::Empty() const {
			return _transactions.empty();
		}

		void GroupedTransactions::UpdateAssets(const AssetIDMap &assetIDMap) {
			_assetIDMap = assetIDMap;
		}

		uint32_t GroupedTransactions::FindAssetTableIDByAssetID(const UInt256 &assetID) const {
			for (AssetIDMap::const_iterator it = _assetIDMap.cbegin(); it != _assetIDMap.cend(); ++it) {
				if (UInt256Eq(&it->second, &assetID))
					return it->first;
			}

			return UINT32_MAX;
		}

		void GroupedTransactions::BatchSet(const boost::function<void(const TransactionPtr &)> &fun) {
			std::for_each(_transactions.begin(), _transactions.end(), fun);
		}

	}
}

