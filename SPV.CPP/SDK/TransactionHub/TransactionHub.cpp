// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionHub.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>
#include <SDK/Common/Utils.h>
#include <SDK/Account/MultiSignSubAccount.h>

#include <Core/BRBIP39Mnemonic.h>
#include <Core/BRArray.h>
#include <Core/BRTransaction.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>
#include <Interface/ISubWallet.h>

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
				tx->isRegistered() = true;
			});

			for (int i = 0; i < txArray.size(); ++i) {
				_txRemarkMap[Utils::UInt256ToString(txArray[i]->getHash(), true)] = txArray[i]->getRemark();
			}
		}

		TransactionHub::~TransactionHub() {
		}

		void TransactionHub::initListeningAddresses(const std::vector<std::string> &addrs) {
			_listeningAddrs = addrs;
			_transactions.InitListeningAddresses(addrs);
		}

		void TransactionHub::RegisterRemark(const TransactionPtr &transaction) {
			_txRemarkMap[Utils::UInt256ToString(transaction->getHash(), true)] = transaction->getRemark();
		}

		std::string TransactionHub::GetRemark(const std::string &txHash) {
			if (_txRemarkMap.find(txHash) == _txRemarkMap.end())
				return "";
			return _txRemarkMap[txHash];
		}

		std::vector<UTXO> TransactionHub::getUTXOsSafe(const UInt256 &assetID) const {
			std::vector<UTXO> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				return _transactions.GetUTXOs(assetID);
			}

			return result;
		}

		std::vector<UTXO> TransactionHub::getAllUTXOsSafe() {
			std::vector<UTXO> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				result = _transactions.GetAllUTXOs();
			}
			return result;
		}

		nlohmann::json TransactionHub::GetBalanceInfo() {
			nlohmann::json info;

			_transactions.ForEach([this, &info](const UInt256 &key, const AssetTransactionsPtr &value) {
				nlohmann::json assetInfo;
				assetInfo["AssetID"] = Utils::UInt256ToString(key, true);
				assetInfo["BalanceInfo"] = value->GetBalanceInfo();
				info.push_back(assetInfo);
			});

			return info;
		}

		uint64_t TransactionHub::GetBalanceWithAddress(const UInt256 &assetID, const Address &address,
													   AssetTransactions::BalanceType type) const {
			std::vector<UTXO> utxos = getUTXOsSafe(assetID);
			uint64_t balance = 0;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = 0; i < utxos.size(); ++i) {
					const TransactionPtr &t = _transactions.Get(assetID)->GetExistTransaction(utxos[i].hash);
					if (t == nullptr || (type == AssetTransactions::Default &&
						t->getOutputs()[utxos[i].n].GetType() != TransactionOutput::Type::Default) ||
						(type == AssetTransactions::Voted &&
						t->getOutputs()[utxos[i].n].GetType() != TransactionOutput::Type::VoteOutput)) {
						continue;
					}

					if (t->getOutputs()[utxos[i].n].GetAddress() == address) {
						balance += t->getOutputs()[utxos[i].n].getAmount();
					}
				}
			}

			return balance;
		}

		uint64_t TransactionHub::getBalance(const UInt256 &assetID, AssetTransactions::BalanceType type) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetBalance(type);
			}
			return result;
		}

		uint64_t TransactionHub::getFeePerKb(const UInt256 &assetID) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetFeePerKb();
			}
			return result;
		}

		void TransactionHub::setFeePerKb(const UInt256 &assetID, uint64_t fee) {
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				_transactions[assetID]->SetFeePerKb(fee);
			}
		}

		uint64_t TransactionHub::getMaxFeePerKb() {
			return MAX_FEE_PER_KB;
		}

		uint64_t TransactionHub::getDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		void TransactionHub::UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress) {
			_transactions.UpdateTxFee(tx, fee, fromAddress);
		}

		TransactionPtr
		TransactionHub::createTransaction(const Address &fromAddress, uint64_t amount,
										  const Address &toAddress, uint64_t fee,
										  const UInt256 &assetID, bool useVotedUTXO) {
			ParamChecker::checkCondition(!toAddress.Valid(), Error::CreateTransaction,
										 "Invalid receiver address " + toAddress.String());

			std::vector<TransactionOutput> outputs;

			outputs.emplace_back(amount, toAddress, assetID);

			return _transactions.CreateTxForFee(outputs, fromAddress, fee, useVotedUTXO);
		}

		bool TransactionHub::containsTransaction(const TransactionPtr &transaction) {
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

		void TransactionHub::RemoveTransaction(const UInt256 &txHash) {
			_transactions.RemoveTransaction(txHash);
		}

		void TransactionHub::UpdateTransactions(const std::vector<UInt256> &txHashes, uint32_t blockHeight,
												uint32_t timestamp) {
			_transactions.UpdateTransactions(txHashes, blockHeight, timestamp);
		}

		TransactionPtr TransactionHub::transactionForHash(const UInt256 &transactionHash) {
			TransactionPtr tx;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				tx = _transactions.TransactionForHash(transactionHash);
			}
			return tx;
		}

		bool TransactionHub::transactionIsValid(const TransactionPtr &transaction) {
			bool r = false;
			if (transaction == nullptr || !transaction->IsSigned()) return r;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

			if (transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be invalid

				boost::mutex::scoped_lock scoped_lock(lock);
				const UInt256 &assetID = transaction->GetAssetID();
				r = _transactions.Get(assetID)->TransactionIsValid(transaction);
			}

			return r;
		}

		bool TransactionHub::transactionIsPending(const TransactionPtr &transaction) {
			time_t now = time(NULL);
			uint32_t height;
			int r = 0;

			assert(transaction->IsSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = _transactions.GetBlockHeight();
			}

			if (transaction != nullptr &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be postdated
				if (transaction->getSize() > TX_MAX_SIZE) r = 1; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->getInputs().size(); i++) {
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX - 1) r = 1; // check for replace-by-fee
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX &&
						transaction->getLockTime() < TX_MAX_LOCK_HEIGHT &&
						transaction->getLockTime() > height + 1)
						r = 1; // future lockTime
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX && transaction->getLockTime() > now)
						r = 1; // future lockTime
				}

				for (size_t i = 0; !r && i < transaction->getOutputs().size(); i++) { // check that no outputs are dust
					if (transaction->getOutputs()[i].getAmount() < TX_MIN_OUTPUT_AMOUNT) r = 1;
				}

				for (size_t i = 0;
					 !r && i < transaction->getInputs().size(); i++) { // check if any inputs are known to be pending
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && transactionIsPending(t)) r = 1;
				}
			}

			return r;
		}

		bool TransactionHub::transactionIsVerified(const TransactionPtr &transaction) {
			bool r = true;
			assert(transaction != NULL && transaction->IsSigned());

			if (transaction &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be unverified
				if (transaction->getTimestamp() == 0 || !transactionIsValid(transaction) ||
					transactionIsPending(transaction))
					r = false;

				for (size_t i = 0;
					 r && i < transaction->getInputs().size(); i++) { // check if any inputs are known to be unverified
					const TransactionPtr &t = transactionForHash(transaction->getInputs()[i].getTransctionHash());
					if (t && !transactionIsVerified(t)) r = false;
				}
			}

			return r;
		}

		uint64_t TransactionHub::getTransactionAmount(const TransactionPtr &tx) {
			uint64_t amountSent = getTransactionAmountSent(tx);
			uint64_t amountReceived = getTransactionAmountReceived(tx);

			return amountSent == 0
				   ? amountReceived
				   : -1 * (amountSent - amountReceived + getTransactionFee(tx));
		}

		uint64_t TransactionHub::getTransactionFee(const TransactionPtr &tx) {
			return WalletFeeForTx(tx);
		}

		uint64_t TransactionHub::getTransactionAmountSent(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = 0; tx && i < tx->getInputs().size(); i++) {
					const TransactionPtr &t = _transactions.TransactionForHash(tx->getInputs()[i].getTransctionHash(),
																			   tx->GetAssetID());
					uint32_t n = tx->getInputs()[i].getIndex();

					if (t && n < t->getOutputs().size() &&
						_subAccount->ContainsAddress(t->getOutputs()[n].GetAddress())) {
						amount += t->getOutputs()[n].getAmount();
					}
				}
			}

			return amount;
		}

		uint64_t TransactionHub::getTransactionAmountReceived(const TransactionPtr &tx) {
			uint64_t amount = 0;

			assert(tx != NULL);
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				// TODO: don't include outputs below TX_MIN_OUTPUT_AMOUNT
				for (size_t i = 0; tx && i < tx->getOutputs().size(); i++) {
					if (_subAccount->ContainsAddress(tx->getOutputs()[i].GetAddress()))
						amount += tx->getOutputs()[i].getAmount();
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

		Address TransactionHub::GetVoteDepositAddress() const {
			if ("Multi-Sign Account" == _subAccount->GetBasicInfo()["Type"]) {
				return Address();
			}

			return Address(_subAccount->GetVotePublicKey(), PrefixDeposit);
		}

		bool TransactionHub::IsVoteDepositAddress(const Address &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->IsDepositAddress(addr);
		}

		bool TransactionHub::containsAddress(const Address &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _subAccount->ContainsAddress(address);
			}
			return result;
		}

		bool TransactionHub::addressIsUsed(const Address &address) {
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
				for (size_t i = 0; i < tx->getInputs().size() && amount != UINT64_MAX; i++) {
					const TransactionPtr &t = _transactions.TransactionForHash(tx->getInputs()[i].getTransctionHash(),
																			   tx->GetAssetID());
					uint32_t n = tx->getInputs()[i].getIndex();

					if (t && n < t->getOutputs().size()) {
						amount += t->getOutputs()[n].getAmount();
					} else amount = UINT64_MAX;
				}
			}

			for (size_t i = 0; tx->getOutputs().size() && amount != UINT64_MAX; i++) {
				amount -= tx->getOutputs()[i].getAmount();
			}

			return amount;
		}

		void TransactionHub::UpdateBalance() {
			_transactions.ForEach([this](const UInt256 &key, const AssetTransactionsPtr &value) {
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
				_transactions.ForEach([&result, &blockHeight](const UInt256 &key, const AssetTransactionsPtr &value) {
					std::vector<TransactionPtr> temp = value->TxUnconfirmedBefore(blockHeight);
					result.insert(result.end(), temp.begin(), temp.end());
				});
			}

			return result;
		}

		const std::vector<std::string> &TransactionHub::getListeningAddrs() const {
			return _listeningAddrs;
		}

		std::vector<Address> TransactionHub::UnusedAddresses(uint32_t gapLimit, bool internal) {
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> TransactionHub::getAllTransactions() const {
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