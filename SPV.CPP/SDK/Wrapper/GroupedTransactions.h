// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
#define __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__

#include <map>

#include <SDK/Transaction/Transaction.h>

namespace Elastos {
	namespace ElaWallet {

		class GroupedTransactions {
		public:
			typedef std::map<uint32_t, UInt256> AssetIDMap;

			const std::vector<TransactionPtr> &GetTransactions(uint32_t assetTableID) const;

			const std::vector<TransactionPtr> &GetAllTransactions() const;

			void SortTransaction();

			void Append(const TransactionPtr &transaction);

			void RemoveAt(size_t index);

			size_t GetSize() const;

			bool Empty() const;

			void BatchSet(const boost::function<void(const TransactionPtr &)> &fun);

			TransactionPtr &operator[](size_t index);

			void UpdateAssets(const AssetIDMap &assetIDMap);

		private:
			uint32_t FindAssetTableIDByAssetID(const UInt256 &assetID) const;

		private:
			mutable std::map<uint32_t, std::vector<TransactionPtr>> _groupedTransactions;
			std::vector<TransactionPtr> _transactions;
			AssetIDMap _assetIDMap;
		};

	}
}


#endif //__ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
