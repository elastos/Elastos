// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDASSET_H__
#define __ELASTOS_SDK__GROUPEDASSET_H__

#include <SDK/Common/ElementSet.h>
#include <SDK/Common/Lockable.h>
#include <SDK/Account/SubAccount.h>
#include <SDK/Common/BigInt.h>

#include <map>
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class Wallet;
		class Asset;
		class Transaction;
		class TransactionOutput;
		class TransactionInput;
		class UTXO;
		typedef boost::shared_ptr<Asset> AssetPtr;
		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef boost::shared_ptr<TransactionInput> InputPtr;

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

			const std::vector<UTXOPtr> &GetUTXOs() const;

			const std::vector<UTXOPtr> &GetCoinBaseUTXOs() const;

			BigInt GetBalance(BalanceType type = Total) const;

			void CleanBalance();

			BigInt UpdateBalance();

			nlohmann::json GetBalanceInfo();

			TransactionPtr CombineUTXO(const std::string &memo, bool useVotedUTXO);

			TransactionPtr CreateTxForOutputs(const std::vector<OutputPtr> &outputs,
											  const Address &fromAddress,
											  const std::string &memo,
											  bool useVotedUTXO,
											  bool autoReduceOutputAmount);

			void AddFeeForTx(TransactionPtr &tx, bool useVotedUTXO);

			const AssetPtr &GetAsset() const;

			void AddUTXO(const UTXOPtr &o);

			void AddCoinBaseUTXO(const UTXOPtr &coinbaseUTXO);

		private:
			uint64_t CalculateFee(uint64_t feePerKB, size_t size);

		private:
			BigInt _balance, _votedBalance, _lockedBalance, _depositBalance;
			std::vector<UTXOPtr> _utxos;
			std::vector<UTXOPtr> _coinBaseUTXOs;

			AssetPtr _asset;

			Wallet *_parent;
		};

		typedef boost::shared_ptr<GroupedAsset> GroupedAssetPtr;

	}
}


#endif //__ELASTOS_SDK__GROUPEDASSET_H__
