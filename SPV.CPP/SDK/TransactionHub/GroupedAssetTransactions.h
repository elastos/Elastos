// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
#define __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__

#include <SDK/TransactionHub/UTXOList.h>
#include <SDK/Common/ElementSet.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Asset.h>

#include <map>
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>
#include <SDK/Common/Lockable.h>
#include <SDK/Account/SubAccount.h>

namespace Elastos {
	namespace ElaWallet {

	class TransactionHub;
	class GroupedAssetTransactions;

	class AssetTransactions {
		public:
			enum BalanceType {
				Default,
				Voted,
				Total,
			};

		public:
			AssetTransactions(GroupedAssetTransactions *parent, Lockable *lockable,
							  const Asset &asset,
							  const SubAccountPtr &subAccount,
							  const std::vector<std::string> &listeningAddrs);

			AssetTransactions(const AssetTransactions &proto);

			AssetTransactions &operator=(const AssetTransactions &proto);

			const std::vector<UTXO> &GetUTXOs() const;

			BigInt GetBalance(BalanceType type = Total) const;

			uint64_t GetFeePerKb() const;

			void SetFeePerKb(uint64_t value);

			void CleanBalance();

			void UpdateBalance();

			nlohmann::json GetBalanceInfo();

			TransactionPtr CreateTxForFee(const std::vector<TransactionOutput> &outputs,
										  const Address &fromAddress,
										  uint64_t fee, bool useVotedUTXO);

			void UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress);

			Asset GetAsset();

			void AddUTXO(const UTXO &o);

		private:
			BigInt _balance, _votedBalance, _lockedBalance, _depositBalance;
			uint64_t _feePerKb;
			UTXOList _utxos, _utxosLocked;

			Lockable *_lockable;
			SubAccountPtr _subAccount;
			std::vector<std::string> _listeningAddrs;

			Asset _asset;

			GroupedAssetTransactions *_parent;
		};

		typedef boost::shared_ptr<AssetTransactions> AssetTransactionsPtr;

		class GroupedAssetTransactions {
		public:
			class Listener {
			public:
				virtual void balanceChanged(const uint256 &asset, const BigInt &balance) = 0;

				virtual void onTxAdded(const TransactionPtr &transaction) = 0;

				virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) = 0;

				virtual void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) = 0;
			};

		public:
			GroupedAssetTransactions(TransactionHub *parent, const std::vector<Asset> &assetArray,
									 const std::vector<TransactionPtr> &txns,
									 const SubAccountPtr &subAccount,
									 const boost::shared_ptr<Listener> &listener);

			void InitListeningAddresses(const std::vector<std::string> &addrs);

			std::vector<TransactionPtr> GetAllTransactions() const;

			const std::vector<UTXO> &GetUTXOs(const uint256 &assetID) const;

			std::vector<UTXO> GetAllUTXOs() const;

			void Append(const TransactionPtr &transaction);

			void BatchSet(const boost::function<void(const TransactionPtr &)> &fun);

			void SortTransaction();

			void ForEach(const boost::function<void(const uint256 &, const AssetTransactionsPtr &)> &fun);

			void RegisterAssets(const std::vector<Asset> &assetArray);

			bool GetAsset(const uint256 &assetID, Asset &asset) const;

			AssetTransactionsPtr &operator[](const uint256 &assetID);

			const AssetTransactionsPtr &Get(const uint256 &assetID) const;

			TransactionPtr CreateTxForFee(const std::vector<TransactionOutput> &outputs, const Address &fromAddress,
										  uint64_t fee, bool useVotedUTXO);

			void UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress);

			std::vector<TransactionPtr> TxUnconfirmedBefore(uint32_t blockHeight);

			void SetTxUnconfirmedAfter(uint32_t blockHeight);

			bool TransactionIsValid(const TransactionPtr &tx);

			bool RegisterTransaction(const TransactionPtr &tx);

			void RemoveTransaction(const uint256 &txHash);

			BigInt AmountSentByTx(const TransactionPtr &tx);

			TransactionPtr TransactionForHash(const uint256 &txHash) const;

			void UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, uint32_t timestamp);

			bool WalletContainsTx(const TransactionPtr &tx);

			void InitWithTransactions(const std::vector<TransactionPtr> &txArray);

			nlohmann::json GetAllSupportedAssets() const;

			bool ContainsAsset(const uint256 &assetID) const;

			uint32_t GetBlockHeight() const {
				return _blockHeight;
			}

			const UTXOList &GetSpentOutputs() const {
				return _spentOutputs;
			}

			const UTXOList &GetSpendingOutputs() const {
				return _spendingOutputs;
			}

			void SetBlockHeight(uint32_t height) {
				_blockHeight = height;
			}

			void CleanBalance();

			void UpdateBalance();

		protected:
			void balanceChanged(const uint256 &asset, const BigInt &balance);

			void txAdded(const TransactionPtr &tx);

			void txUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, uint32_t timestamp);

			void txDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan);

		private:
			uint256 GetUniqueAssetID(const std::vector<TransactionOutput> &outputs) const;

		private:
			typedef std::map<uint256, AssetTransactionsPtr> AssetTransactionMap;
			mutable AssetTransactionMap _groupedTransactions;

			typedef ElementSet<TransactionPtr> TransactionSet;
			std::vector<TransactionPtr> _transactions;
			TransactionSet _allTx, _invalidTx;
			UTXOList _spentOutputs, _spendingOutputs;

			uint32_t _blockHeight;
			TransactionHub *_parent;
			SubAccountPtr _subAccount;
			std::vector<std::string> _listeningAddrs;
			boost::weak_ptr<Listener> _listener;
		};

	}
}


#endif //__ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
