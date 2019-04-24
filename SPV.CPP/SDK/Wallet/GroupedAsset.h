// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
#define __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__

#include <SDK/Wallet/UTXOList.h>
#include <SDK/Common/ElementSet.h>
#include <SDK/Common/Lockable.h>
#include <SDK/Account/SubAccount.h>

#include <map>
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

	class Wallet;
	class Asset;
	class Transaction;
	class TransactionOutput;
	typedef boost::shared_ptr<Asset> AssetPtr;

	class GroupedAsset {
		public:
			enum BalanceType {
				Default,
				Voted,
				Total,
			};

		public:
			GroupedAsset();

			GroupedAsset(Wallet *parent, const AssetPtr &asset);

			GroupedAsset(const GroupedAsset &proto);

			GroupedAsset &operator=(const GroupedAsset &proto);

			const std::vector<UTXO> &GetUTXOs() const;

			BigInt GetBalance(BalanceType type = Total) const;

			void CleanBalance();

			BigInt UpdateBalance();

			nlohmann::json GetBalanceInfo();

			TransactionPtr CreateTxForFee(const std::vector<TransactionOutput> &outputs,
										  const Address &fromAddress,
										  uint64_t fee, bool useVotedUTXO);

			void UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress);

			const AssetPtr &GetAsset() const;

			void AddUTXO(const UTXO &o);

		private:
			BigInt _balance, _votedBalance, _lockedBalance, _depositBalance;
			UTXOList _utxos, _utxosLocked;

			AssetPtr _asset;

			Wallet *_parent;
		};

		typedef boost::shared_ptr<GroupedAsset> GroupedAssetPtr;

	}
}


#endif //__ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
