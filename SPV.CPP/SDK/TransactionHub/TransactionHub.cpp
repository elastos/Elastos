// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionHub.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>
#include <SDK/Common/Utils.h>
#include <SDK/Account/MultiSignSubAccount.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>

#include <Interface/ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		TransactionHub::TransactionHub(const std::vector<Asset> &assetArray,
						const std::vector<TransactionPtr> &txArray,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<AssetTransactions::Listener> &listener) :
				_subAccount(subAccount),
				_transactions(this, assetArray, txArray, subAccount, listener) {

			_subAccount->InitAccount(txArray, this);

			_transactions.InitWithTransactions(txArray);

			_transactions.BatchSet([](const TransactionPtr &tx) {
				tx->IsRegistered() = true;
			});

			for (int i = 0; i < txArray.size(); ++i) {
				_txRemarkMap[txArray[i]->GetHash().GetHex()] = txArray[i]->GetRemark();
			}
		}

		TransactionHub::~TransactionHub() {
		}

		void TransactionHub::InitListeningAddresses(const std::vector<std::string> &addrs) {
			_listeningAddrs = addrs;
			_transactions.InitListeningAddresses(addrs);
		}

		void TransactionHub::RegisterRemark(const TransactionPtr &transaction) {
			_txRemarkMap[transaction->GetHash().GetHex()] = transaction->GetRemark();
		}

		std::string TransactionHub::GetRemark(const std::string &txHash) {
			if (_txRemarkMap.find(txHash) == _txRemarkMap.end())
				return "";
			return _txRemarkMap[txHash];
		}

		std::vector<UTXO> TransactionHub::GetUTXOsSafe(const uint256 &assetID) const {
			std::vector<UTXO> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				return _transactions.GetUTXOs(assetID);
			}

			return result;
		}

		std::vector<UTXO> TransactionHub::GetAllUTXOsSafe() {
			std::vector<UTXO> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				result = _transactions.GetAllUTXOs();
			}
			return result;
		}

		nlohmann::json TransactionHub::GetBalanceInfo() {
			nlohmann::json info;
			boost::mutex::scoped_lock scopedLock(lock);

			_transactions.ForEach([this, &info](const uint256 &key, const AssetTransactionsPtr &value) {
				nlohmann::json assetInfo;
				assetInfo["AssetID"] = key.GetHex();
				assetInfo["Summary"] = value->GetBalanceInfo();
				info.push_back(assetInfo);
			});

			return info;
		}

		uint64_t TransactionHub::GetBalanceWithAddress(const uint256 &assetID, const Address &address,
													   AssetTransactions::BalanceType type) const {
			std::vector<UTXO> utxos = GetUTXOsSafe(assetID);
			uint64_t balance = 0;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = 0; i < utxos.size(); ++i) {
					const TransactionPtr &t = _transactions.Get(assetID)->GetExistTransaction(utxos[i].hash);
					if (t == nullptr || (type == AssetTransactions::Default &&
						t->GetOutputs()[utxos[i].n].GetType() != TransactionOutput::Type::Default) ||
						(type == AssetTransactions::Voted &&
							t->GetOutputs()[utxos[i].n].GetType() != TransactionOutput::Type::VoteOutput)) {
						continue;
					}

					if (t->GetOutputs()[utxos[i].n].GetAddress() == address) {
						balance += t->GetOutputs()[utxos[i].n].GetAmount();
					}
				}
			}

			return balance;
		}

		uint64_t TransactionHub::GetBalance(const uint256 &assetID, AssetTransactions::BalanceType type) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetBalance(type);
			}
			return result;
		}

		uint64_t TransactionHub::GetFeePerKb(const uint256 &assetID) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetFeePerKb();
			}
			return result;
		}

		void TransactionHub::SetFeePerKb(const uint256 &assetID, uint64_t fee) {
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				_transactions[assetID]->SetFeePerKb(fee);
			}
		}

		uint64_t TransactionHub::GetDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		void TransactionHub::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress) {
			_transactions.UpdateTxFee(tx, fee, fromAddress);
		}

		TransactionPtr
		TransactionHub::CreateTransaction(const Address &fromAddress, uint64_t amount,
										  const Address &toAddress, uint64_t fee,
										  const uint256 &assetID, bool useVotedUTXO) {
			ErrorChecker::CheckCondition(!toAddress.Valid(), Error::CreateTransaction,
										 "Invalid receiver address " + toAddress.String());

			std::vector<TransactionOutput> outputs;

			outputs.emplace_back(amount, toAddress, assetID);

			return _transactions.CreateTxForFee(outputs, fromAddress, fee, useVotedUTXO);
		}

		bool TransactionHub::ContainsTransaction(const TransactionPtr &transaction) {
			bool result = false;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.WalletContainsTx(transaction);
			}
			return result;
		}

		bool TransactionHub::RegisterTransaction(const TransactionPtr &tx) {
			return _transactions.RegisterTransaction(tx);
		}

		void TransactionHub::RemoveTransaction(const uint256 &txHash) {
			_transactions.RemoveTransaction(txHash);
		}

		void TransactionHub::UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight,
												uint32_t timestamp) {
			_transactions.UpdateTransactions(txHashes, blockHeight, timestamp);
		}

		TransactionPtr TransactionHub::TransactionForHash(const uint256 &transactionHash) {
			TransactionPtr tx;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				tx = _transactions.TransactionForHash(transactionHash);
			}
			return tx;
		}

		bool TransactionHub::TransactionIsValid(const TransactionPtr &transaction) {
			bool r = false;
			if (transaction == nullptr || !transaction->IsSigned()) return r;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

			if (transaction->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be invalid

				boost::mutex::scoped_lock scoped_lock(lock);
				const uint256 &assetID = transaction->GetAssetID();
				r = _transactions.Get(assetID)->TransactionIsValid(transaction);
			}

			return r;
		}

#if 0
		bool TransactionHub::TransactionIsPending(const TransactionPtr &transaction) {
			time_t now = time(NULL);
			uint32_t height;
			int r = 0;

			assert(transaction->IsSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = _transactions.GetBlockHeight();
			}

			if (transaction != nullptr &&
				transaction->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be postdated
				if (transaction->GetSize() > TX_MAX_SIZE) r = 1; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->GetInputs().size(); i++) {
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX - 1) r = 1; // check for replace-by-fee
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX &&
						transaction->GetLockTime() < TX_MAX_LOCK_HEIGHT &&
						transaction->GetLockTime() > height + 1)
						r = 1; // future lockTime
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX && transaction->GetLockTime() > now)
						r = 1; // future lockTime
				}

				for (size_t i = 0; !r && i < transaction->GetOutputs().size(); i++) { // check that no outputs are dust
					if (transaction->GetOutputs()[i].GetAmount() < TX_MIN_OUTPUT_AMOUNT) r = 1;
				}

				for (size_t i = 0;
					 !r && i < transaction->GetInputs().size(); i++) { // check if any inputs are known to be pending
					const TransactionPtr &t = TransactionForHash(transaction->GetInputs()[i].GetTransctionHash());
					if (t && TransactionIsPending(t)) r = 1;
				}
			}

			return r;
		}

		bool TransactionHub::TransactionIsVerified(const TransactionPtr &transaction) {
			bool r = true;
			assert(transaction != NULL && transaction->IsSigned());

			if (transaction &&
				transaction->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be unverified
				if (transaction->GetTimestamp() == 0 || !TransactionIsValid(transaction) ||
					TransactionIsPending(transaction))
					r = false;

				for (size_t i = 0;
					 r && i < transaction->GetInputs().size(); i++) { // check if any inputs are known to be unverified
					const TransactionPtr &t = TransactionForHash(transaction->GetInputs()[i].GetTransctionHash());
					if (t && !TransactionIsVerified(t)) r = false;
				}
			}

			return r;
		}
#endif

		uint64_t TransactionHub::GetTransactionAmount(const TransactionPtr &tx) {
			uint64_t amountSent = GetTransactionAmountSent(tx);
			uint64_t amountReceived = GetTransactionAmountReceived(tx);

			return amountSent == 0
				   ? amountReceived
				   : -1 * (amountSent - amountReceived + GetTransactionFee(tx));
		}

		uint64_t TransactionHub::GetTransactionFee(const TransactionPtr &tx) {
			return WalletFeeForTx(tx);
		}

		uint64_t TransactionHub::GetTransactionAmountSent(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; tx && i < tx->GetInputs().size(); i++) {
					const TransactionPtr &t = _transactions.TransactionForHash(tx->GetInputs()[i].GetTransctionHash(),
																			   tx->GetAssetID());
					uint32_t n = tx->GetInputs()[i].GetIndex();

					if (t && n < t->GetOutputs().size() &&
						_subAccount->ContainsAddress(t->GetOutputs()[n].GetAddress())) {
						amount += t->GetOutputs()[n].GetAmount();
					}
				}
			}

			return amount;
		}

		uint64_t TransactionHub::GetTransactionAmountReceived(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				// TODO: don't include outputs below TX_MIN_OUTPUT_AMOUNT
				for (size_t i = 0; tx && i < tx->GetOutputs().size(); i++) {
					if (_subAccount->ContainsAddress(tx->GetOutputs()[i].GetAddress()))
						amount += tx->GetOutputs()[i].GetAmount();
				}
			}

			return amount;
		}

		Address TransactionHub::GetReceiveAddress() const {
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0];
		}

		size_t TransactionHub::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) {
			return _subAccount->GetAllAddresses(addr, start, count, containInternal);
		}

		Address TransactionHub::GetOwnerDepositAddress() const {
			if ("Multi-Sign Account" == _subAccount->GetBasicInfo()["Type"]) {
				return Address();
			}

			return Address(PrefixDeposit, _subAccount->GetOwnerPublicKey());
		}

		Address TransactionHub::GetOwnerAddress() const {
			if ("Multi-Sign Account" == _subAccount->GetBasicInfo()["Type"]) {
				return Address();
			}

			return Address(PrefixStandard, _subAccount->GetOwnerPublicKey());
		}

		bool TransactionHub::IsVoteDepositAddress(const Address &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->IsDepositAddress(addr);
		}

		bool TransactionHub::ContainsAddress(const Address &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _subAccount->ContainsAddress(address);
			}
			return result;
		}

		bool TransactionHub::AddressIsUsed(const Address &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _subAccount->IsAddressUsed(address);
			}
			return result;
		}

		uint64_t TransactionHub::WalletFeeForTx(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != nullptr);
			if (tx == nullptr) {
				return amount;
			}

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; i < tx->GetInputs().size() && amount != UINT64_MAX; i++) {
					const TransactionPtr &t = _transactions.TransactionForHash(tx->GetInputs()[i].GetTransctionHash(),
																			   tx->GetAssetID());
					uint32_t n = tx->GetInputs()[i].GetIndex();

					if (t && n < t->GetOutputs().size()) {
						amount += t->GetOutputs()[n].GetAmount();
					} else amount = UINT64_MAX;
				}
			}

			for (size_t i = 0; tx->GetOutputs().size() && amount != UINT64_MAX; i++) {
				amount -= tx->GetOutputs()[i].GetAmount();
			}

			return amount;
		}

		void TransactionHub::UpdateBalance() {
			_transactions.ForEach([this](const uint256 &key, const AssetTransactionsPtr &value) {
				value->UpdateBalance();
			});
		}

		const std::string &TransactionHub::GetWalletID() const {
			return _walletID;
		}

		void TransactionHub::SetWalletID(const std::string &walletID) {
			_walletID = walletID;
		}

		void TransactionHub::SetBlockHeight(uint32_t height) {
			boost::mutex::scoped_lock scopedLock(lock);
			_transactions.SetBlockHeight(height);
		}

		uint32_t TransactionHub::GetBlockHeight() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _transactions.GetBlockHeight();
		}

		void TransactionHub::SignTransaction(const TransactionPtr &tx, const std::string &payPassword) {
			_subAccount->SignTransaction(tx, payPassword);
		}

		std::vector<TransactionPtr> TransactionHub::TxUnconfirmedBefore(uint32_t blockHeight) {
			std::vector<TransactionPtr> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				_transactions.ForEach([&result, &blockHeight](const uint256 &key, const AssetTransactionsPtr &value) {
					std::vector<TransactionPtr> temp = value->TxUnconfirmedBefore(blockHeight);
					result.insert(result.end(), temp.begin(), temp.end());
				});
			}

			return result;
		}

		const std::vector<std::string> &TransactionHub::GetListeningAddrs() const {
			return _listeningAddrs;
		}

		std::vector<Address> TransactionHub::UnusedAddresses(uint32_t gapLimit, bool internal) {
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> TransactionHub::GetAllTransactions() const {
			std::vector<TransactionPtr> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				result = _transactions.GetAllTransactions();
			}
			return result;
		}

		void TransactionHub::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			_transactions.SetTxUnconfirmedAfter(blockHeight);
		}

		void TransactionHub::UpdateAssets(const std::vector<Asset> &assetArray) {
			boost::mutex::scoped_lock scopedLock(lock);
			_transactions.UpdateAssets(assetArray);
		}

		nlohmann::json TransactionHub::GetAllSupportedAssets() const {
			return _transactions.GetAllSupportedAssets();
		}

		bool TransactionHub::ContainsAsset(const std::string &assetID) {
			return _transactions.ContainsAsset(assetID);
		}

	}
}