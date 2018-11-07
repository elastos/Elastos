// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdlib.h>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <SDK/Common/Log.h>
#include <Core/BRAddress.h>
#include <Core/BRBIP32Sequence.h>
#include <SDK/Common/ParamChecker.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/Payload/PayloadRegisterAsset.h>

#include "BRAddress.h"
#include "BRBIP39Mnemonic.h"
#include "BRArray.h"
#include "BRTransaction.h"

#include "TransactionHub.h"
#include "Utils.h"
#include "Account/MultiSignSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionHub::TransactionHub(const std::vector<TransactionPtr> &txArray,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Listener> &listener) :
				_subAccount(subAccount),
				_transactions(this, subAccount) {

			_transactions.InitWithTransactions(txArray);

			_subAccount->InitAccount(txArray, this);
			UpdateBalance();

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			_transactions.BatchSet([](const TransactionPtr &tx) {
				tx->isRegistered() = true;
			});

			for (int i = 0; i < txArray.size(); ++i) {
				_txRemarkMap[Utils::UInt256ToString(txArray[i]->getHash())] = txArray[i]->getRemark();
			}
		}

		TransactionHub::~TransactionHub() {
		}

		void TransactionHub::initListeningAddresses(const std::vector<std::string> &addrs) {
			_listeningAddrs = addrs;
			_transactions.InitListeningAddresses(addrs);
		}

		void TransactionHub::RegisterRemark(const TransactionPtr &transaction) {
			_txRemarkMap[Utils::UInt256ToString(transaction->getHash())] = transaction->getRemark();
		}

		std::string TransactionHub::GetRemark(const std::string &txHash) {
			if (_txRemarkMap.find(txHash) != _txRemarkMap.end())
				return "";
			return _txRemarkMap[txHash];
		}

		std::vector<UTXO> TransactionHub::getUTXOsSafe(const UInt256 &assetID) {
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

		nlohmann::json TransactionHub::GetBalanceInfo(const UInt256 &assetID) {
			std::vector<UTXO> _utxos = getUTXOsSafe(assetID);
			nlohmann::json j;
			std::map<std::string, uint64_t> addressesBalanceMap;

			{
				boost::mutex::scoped_lock scopedLock(lock);

				for (size_t i = 0; i < _utxos.size(); ++i) {
					if (!_transactions[assetID]->Exist(_utxos[i].hash)) continue;

					const TransactionPtr &t = _transactions[assetID]->GetExistTransaction(_utxos[i].hash);
					if (addressesBalanceMap.find(t->getOutputs()[_utxos[i].n].getAddress()) !=
						addressesBalanceMap.end()) {
						addressesBalanceMap[t->getOutputs()[_utxos[i].n].getAddress()] += t->getOutputs()[_utxos[i].n].getAmount();
					} else {
						addressesBalanceMap[t->getOutputs()[_utxos[i].n].getAddress()] = t->getOutputs()[_utxos[i].n].getAmount();
					}
				}
			}

			std::vector<nlohmann::json> balances;
			std::for_each(addressesBalanceMap.begin(), addressesBalanceMap.end(),
						  [&addressesBalanceMap, &balances](const std::map<std::string, uint64_t>::value_type &item) {
							  nlohmann::json balanceKeyValue;
							  balanceKeyValue[item.first] = item.second;
							  balances.push_back(balanceKeyValue);
						  });

			j["Balances"] = balances;
			return j;
		}

		uint64_t TransactionHub::GetBalanceWithAddress(const UInt256 &assetID, const std::string &address) {
			std::vector<UTXO> utxos = getUTXOsSafe(assetID);
			uint64_t balance = 0;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = 0; i < utxos.size(); ++i) {
					const TransactionPtr &t = _transactions[assetID]->GetExistTransaction(utxos[i].hash);
					if (t == nullptr) continue;
					if (t->getOutputs()[utxos[i].n].getAddress() == address) {
						balance += t->getOutputs()[utxos[i].n].getAmount();
					}
				}
			}

			return balance;
		}

		uint64_t TransactionHub::getBalance(const UInt256 &assetID) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetBalance();
			}
			return result;
		}

		uint64_t TransactionHub::getTotalSent(const UInt256 &assetID) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetTotalSent();
			}
			return result;
		}

		uint64_t TransactionHub::getTotalReceived(const UInt256 &assetID) const {
			uint64_t result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.Get(assetID)->GetTotalReceived();
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

		bool TransactionHub::AddressFilter(const std::string &fromAddress, const std::string &filterAddress) {
			return filterAddress == fromAddress;
		}

		TransactionPtr
		TransactionHub::createTransaction(const std::string &fromAddress, uint64_t fee, uint64_t amount,
								  const std::string &toAddress, const UInt256 &assetID, const std::string &remark,
								  const std::string &memo) {
			UInt168 u168Address = UINT168_ZERO;
			ParamChecker::checkCondition(!fromAddress.empty() && !Utils::UInt168FromAddress(u168Address, fromAddress),
										 Error::CreateTransaction, "Invalid spender address " + fromAddress);

			ParamChecker::checkCondition(!Utils::UInt168FromAddress(u168Address, toAddress), Error::CreateTransaction,
										 "Invalid receiver address " + toAddress);

			TransactionOutput output(amount, toAddress);

			std::vector<TransactionOutput> outputs = {output};
			TransactionPtr result = _transactions.CreateTxForOutputs(outputs, fromAddress,
																	 boost::bind(&TransactionHub::AddressFilter, this, _1, _2));
			if (result != nullptr) {
				result->setRemark(remark);

				result->addAttribute(
						Attribute(Attribute::Nonce, Utils::convertToMemBlock(std::to_string(std::rand()))));
				if (!memo.empty())
					result->addAttribute(Attribute(Attribute::Memo, Utils::convertToMemBlock(memo)));
				if (result->getTransactionType() == Transaction::TransferCrossChainAsset)
					result->addAttribute(
							Attribute(Attribute::Confirmations, Utils::convertToMemBlock(std::to_string(1))));
			}

			return result;
		}

		bool TransactionHub::containsTransaction(const TransactionPtr &transaction) {
			bool result = false;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _transactions.WalletContainsTx(transaction);
			}
			return result;
		}

		bool TransactionHub::registerTransaction(const TransactionPtr &transaction) {
			bool wasAdded = false, r = true;

			assert(transaction != nullptr && transaction->isSigned());

			if (transaction != nullptr && transaction->isSigned()) {
				boost::mutex::scoped_lock scopedLock(lock);
				r = _transactions[transaction->GetAssetID()]->RegisterTransaction(transaction, _blockHeight, wasAdded);
			} else r = false;

			if (wasAdded) {
				// when a wallet address is used in a transaction, generate a new address to replace it
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				balanceChanged(_balance);
				txAdded(transaction);
			}

			return r;
		}

		void TransactionHub::removeTransaction(const UInt256 &transactionHash) {
			bool notifyUser = false, recommendRescan = false;
			UInt256 removedAssetID;
			std::vector<UInt256> removedTransactions;

			assert(!UInt256IsZero(&transactionHash));

			{
				boost::mutex::scoped_lock scopedLock(lock);
				_transactions.RemoveTransaction(transactionHash, _blockHeight, removedTransactions, removedAssetID,
												notifyUser, recommendRescan);
			}

			balanceChanged();
			txDeleted(removedTransactions, removedAssetID, notifyUser, recommendRescan);
		}

		void TransactionHub::updateTransactions(const std::vector<UInt256> &transactionsHashes, uint32_t height,
										uint32_t timestamp) {
			std::vector<UInt256> hashes;
			int needsUpdate = 0;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (height > _blockHeight) _blockHeight = height;
				hashes = _transactions.UpdateTransactions(transactionsHashes, height, _blockHeight, timestamp);
			}

			if (!hashes.empty()) txUpdated(hashes, _blockHeight, timestamp);
			if (needsUpdate) balanceChanged();
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
			if (transaction == nullptr || !transaction->isSigned()) return r;

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

			assert(transaction->isSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = _blockHeight;
			}

			if (transaction != nullptr &&
				transaction->getBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be postdated
				if (transaction->getSize() > TX_MAX_SIZE) r = 1; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->getInputs().size(); i++) {
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX - 1) r = 1; // check for replace-by-fee
					if (transaction->getInputs()[i].getSequence() < UINT32_MAX &&
						transaction->getLockTime() < TX_MAX_LOCK_HEIGHT &&
						transaction->getLockTime() > _blockHeight + 1)
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
			assert(transaction != NULL && transaction->isSigned());

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
						_subAccount->ContainsAddress(t->getOutputs()[n].getAddress())) {
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
					if (_subAccount->ContainsAddress(tx->getOutputs()[i].getAddress()))
						amount += tx->getOutputs()[i].getAmount();
				}
			}

			return amount;
		}

		uint64_t TransactionHub::getBalanceAfterTransaction(const TransactionPtr &transaction) {
			return BalanceAfterTx(transaction);
		}

		std::string TransactionHub::getReceiveAddress() const {
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0].stringify();
		}

		std::vector<std::string> TransactionHub::getAllAddresses() {

			std::vector<Address> addrs = _subAccount->GetAllAddresses(INT64_MAX);

			std::vector<std::string> results;
			for (int i = 0; i < addrs.size(); i++) {
				results.push_back(addrs[i].stringify());
			}
			return results;
		}

		bool TransactionHub::containsAddress(const std::string &address) {
			bool result;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				result = _subAccount->ContainsAddress(address);
			}
			return result;
		}

		bool TransactionHub::addressIsUsed(const std::string &address) {
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
			_subAccount->ClearUsedAddresses();
			_transactions.ForEach([this](const UInt256 &key, const AssetTransactionsPtr &value) {
				value->UpdateBalance(_blockHeight);
			});
		}

		void TransactionHub::balanceChanged() {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged();
			}
		}

		void TransactionHub::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void TransactionHub::txUpdated(const std::vector<UInt256> &txHashes, uint32_t _blockHeight, uint32_t timestamp) {
			if (!_listener.expired()) {
				// Invoke the callback for each of txHashes.
				for (size_t i = 0; i < txHashes.size(); i++) {
					_listener.lock()->onTxUpdated(Utils::UInt256ToString(txHashes[i]), _blockHeight, timestamp);
				}
			}
		}

		void TransactionHub::txDeleted(const std::vector<UInt256> &txHashes, const UInt256 &assetID, bool notifyUser,
							   bool recommendRescan) {
			if (!_listener.expired()) {

				for (size_t i = 0; i < txHashes.size(); i++) {
					_listener.lock()->onTxDeleted(Utils::UInt256ToString(txHashes[i]),
												  UInt256IsZero(&assetID) ? "" : Utils::UInt256ToString(assetID),
												  notifyUser, recommendRescan);
				}
			}
		}

		uint32_t TransactionHub::getBlockHeight() const {
			return _blockHeight;
		}

		uint64_t TransactionHub::BalanceAfterTx(const TransactionPtr &tx) {
			uint64_t result;

			assert(tx != NULL && tx->isSigned());
			{
				boost::mutex::scoped_lock scoped_lock(lock);

				const UInt256 &assetID = tx->GetAssetID();
				result = _transactions.Get(assetID)->GetBalance();
				const std::vector<TransactionPtr> &transactions = _transactions.Get(assetID)->GetTransactions();
				for (size_t i = transactions.size(); i > 0; i--) {
					if (!tx->IsEqual(transactions[i - 1].get())) continue;
					result = _transactions.Get(assetID)->GetBalanceHistory()[i - 1];
					break;
				}
			}

			return result;
		}

		void TransactionHub::signTransaction(const TransactionPtr &transaction, const std::string &payPassword) {
			_subAccount->SignTransaction(transaction, shared_from_this(), payPassword);
		}

		std::vector<TransactionPtr> TransactionHub::TxUnconfirmedBefore(uint32_t blockHeight) {
			std::vector<TransactionPtr> result;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				_transactions.ForEach([&result, blockHeight](const UInt256 &key, const AssetTransactionsPtr &value) {
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
			size_t count;
			std::vector<UInt256> hashes;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				_blockHeight = blockHeight;
				_transactions.ForEach([&hashes, blockHeight](const UInt256 &key, const AssetTransactionsPtr &value) {
					std::vector<UInt256> temp = value->SetTxUnconfirmedAfter(blockHeight);
					hashes.insert(hashes.end(), temp.begin(), temp.end());
				});
			}

			if (count > 0)
				txUpdated(hashes, TX_UNCONFIRMED, 0);
				balanceChanged();
			}
		}

		void TransactionHub::UpdateAssets(const UInt256ValueMap<std::string> &assetIDMap) {
			boost::mutex::scoped_lock scopedLock(lock);
			_transactions.UpdateAssets(assetIDMap);
		}

		nlohmann::json TransactionHub::GetAllSupportedAssets() const {
			return _transactions.GetAllSupportedAssets();
		}

		bool TransactionHub::ContainsAsset(const std::string &assetID) {
			return _transactions.ContainsAsset(assetID);
		}

	}
}