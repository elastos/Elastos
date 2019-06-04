// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"

#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>

#include <Interface/ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		Wallet::Wallet(const std::vector<AssetPtr> &assetArray,
					   const std::vector<TransactionPtr> &txns,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Wallet::Listener> &listener) :
				_feePerKb(DEFAULT_FEE_PER_KB) {

			_subAccount = subAccount;
			for (size_t i = 0; i < txns.size(); ++i) {
				if (!txns[i]->IsSigned() || _allTx.Contains(txns[i])) continue;
				_allTx.Insert(txns[i]);
				InsertTx(txns[i]);
			}

			_subAccount->Init(txns, this);

			if (!txns.empty() && !ContainsTx(txns[0])) { // verify _transactions match master pubKey
				std::string hash = txns[0]->GetHash().GetHex();
				ErrorChecker::ThrowLogicException(Error::WalletNotContainTx, "Wallet do not contain tx = " + hash);
			}


			for (size_t i = 0; i < _transactions.size(); ++i) {
				_transactions[i]->IsRegistered() = true;
			}

			_listener = boost::weak_ptr<Listener>(listener);

			if (assetArray.empty()) {
				InstallDefaultAsset();
			} else {
				InstallAssets(assetArray);
			}
		}

		Wallet::~Wallet() {
		}

		void Wallet::InitListeningAddresses(const std::vector<std::string> &addrs) {
			boost::mutex::scoped_lock scopedLock(lock);
			_listeningAddrs = addrs;
		}

		std::vector<UTXO> Wallet::GetAllUTXO() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<UTXO> result;

			GroupedAssetMap::iterator it;
			for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				result.insert(result.end(), it->second->GetUTXOs().begin(), it->second->GetUTXOs().end());
			}

			return result;
		}

		nlohmann::json Wallet::GetBalanceInfo() {
			boost::mutex::scoped_lock scopedLock(lock);
			nlohmann::json info;

			GroupedAssetMap::iterator it;
			for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				nlohmann::json assetInfo;
				assetInfo["AssetID"] = it->first.GetHex();
				assetInfo["Summary"] = it->second->GetBalanceInfo();
				info.push_back(assetInfo);
			}

			return info;
		}

		BigInt Wallet::GetBalanceWithAddress(const uint256 &assetID, const Address &address,
											 GroupedAsset::BalanceType type) const {
			boost::mutex::scoped_lock scopedLock(lock);

			BigInt balance = 0;
			std::vector<UTXO> utxos = GetUTXO(assetID);

			for (size_t i = 0; i < utxos.size(); ++i) {
				const TransactionPtr tx = _allTx.Get(utxos[i].hash);
				if (tx == nullptr ||
					(type == GroupedAsset::Default &&
						tx->GetOutputs()[utxos[i].n].GetType() != TransactionOutput::Type::Default) ||
					(type == GroupedAsset::Voted &&
						tx->GetOutputs()[utxos[i].n].GetType() != TransactionOutput::Type::VoteOutput)) {
					continue;
				}

				if (tx->GetOutputs()[utxos[i].n].GetAddress() == address) {
					balance += tx->GetOutputs()[utxos[i].n].GetAmount();
				}
			}

			return balance;
		}

		BigInt Wallet::GetBalance(const uint256 &assetID, GroupedAsset::BalanceType type) const {
			ErrorChecker::CheckParam(!ContainsAsset(assetID), Error::InvalidAsset, "asset not found");

			boost::mutex::scoped_lock scoped_lock(lock);

			return _groupedAssets[assetID]->GetBalance(type);
		}

		uint64_t Wallet::GetFeePerKb() const {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _feePerKb;
		}

		void Wallet::SetFeePerKb(uint64_t fee) {
			boost::mutex::scoped_lock scoped_lock(lock);
			_feePerKb = fee;
		}

		uint64_t Wallet::GetDefaultFeePerKb() {
			return DEFAULT_FEE_PER_KB;
		}

		TransactionPtr Wallet::CreateTransaction(const Address &fromAddress,
												 const std::vector<TransactionOutput> &outputs,
												 bool useVotedUTXO, bool autoReduceOutputAmount) {

			uint256 assetID = outputs.front().GetAssetID();

			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!IsAssetUnique(outputs), Error::InvalidAsset, "asset is not unique in outputs");
			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->CreateTxForOutputs(outputs, fromAddress, useVotedUTXO,
																			autoReduceOutputAmount);

			if (assetID != Asset::GetELAAssetID())
				_groupedAssets[Asset::GetELAAssetID()]->AddFeeForTx(tx, useVotedUTXO);

			return tx;
		}

		bool Wallet::ContainsTransaction(const TransactionPtr &tx) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return ContainsTx(tx);
		}

		bool Wallet::RegisterTransaction(const TransactionPtr &tx) {
			bool r = true, wasAdded = false;
			std::map<uint256, BigInt> changedBalance;
			std::map<uint256, BigInt>::iterator it;

			if (tx != nullptr && tx->IsSigned()) {
				Lock();
				if (!_allTx.Contains(tx)) {
					if (ContainsTx(tx)) {
						// TODO: verify signatures when possible
						// TODO: handle tx replacement with input sequence numbers
						//       (for now, replacements appear invalid until confirmation)
						_allTx.Insert(tx);
						InsertTx(tx);
						if (tx->GetBlockHeight() != TX_UNCONFIRMED && tx->GetTransactionType() != Transaction::CoinBase) {
							changedBalance = UpdateBalanceInternal();
						}
						wasAdded = true;
					} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
						// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
						if (tx->GetBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(tx);
						r = false;
						// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
					}
				}
				Unlock();
			} else {
				r = false;
			}

			if (wasAdded) {
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				txAdded(tx);
				if (tx->GetBlockHeight() != TX_UNCONFIRMED && tx->GetTransactionType() != Transaction::CoinBase)
					for (it = changedBalance.begin(); it != changedBalance.end(); ++it)
						balanceChanged(it->first, it->second);
			}

			return r;
		}

		void Wallet::RemoveTransaction(const uint256 &txHash) {
			bool notifyUser = false, recommendRescan = false;
			std::vector<uint256> hashes;
			std::map<uint256, BigInt> changedBalance;
			std::map<uint256, BigInt>::iterator it;

			assert(txHash != 0);

			Lock();
			const TransactionPtr tx = _allTx.Get(txHash);

			if (tx) {
				for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
					const TransactionPtr &t = _transactions[i - 1];
					if (t->GetBlockHeight() < tx->GetBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->GetInputs().size(); j++) {
						if (t->GetInputs()[j].GetTransctionHash() != txHash) continue;
						hashes.push_back(t->GetHash());
						break;
					}
				}

				if (!hashes.empty()) {
					Unlock();
					for (size_t i = hashes.size(); i > 0; i--) {
						RemoveTransaction(hashes[i - 1]);
					}

					RemoveTransaction(txHash);
				} else {
					for (size_t i = _transactions.size(); i > 0; i--) {
						if (!_transactions[i - 1]->IsEqual(tx.get())) continue;
						_transactions.erase(_transactions.begin() + i - 1);
						break;
					}

					changedBalance = UpdateBalanceInternal();
					Unlock();

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && TransactionIsValid(tx)) {
						recommendRescan = notifyUser = true;

						for (size_t i = 0;
							 i < tx->GetInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							TransactionPtr t = TransactionForHash(tx->GetInputs()[i].GetTransctionHash());
							if (t && t->GetBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = false;
							break;
						}
					}

					txDeleted(tx->GetHash(), notifyUser, recommendRescan);
					for (it = changedBalance.begin(); it != changedBalance.end(); ++it)
						balanceChanged(it->first, it->second);
				}
			} else {
				Unlock();
			}
		}

		void Wallet::UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, uint32_t timestamp) {
			std::vector<uint256> hashes;
			bool needsUpdate = false;
			std::map<uint256, BigInt> changedBalance;
			std::vector<PayloadRegisterAsset *> payloads;
			size_t i;

			Lock();
			if (blockHeight != TX_UNCONFIRMED && blockHeight > _blockHeight)
				_blockHeight = blockHeight;

			for (i = 0; i < txHashes.size(); i++) {
				const TransactionPtr tx = _allTx.Get(txHashes[i]);
				if (tx == nullptr || (tx->GetBlockHeight() == blockHeight && tx->GetTimestamp() == timestamp))
					continue;

				if (tx->GetBlockHeight() == TX_UNCONFIRMED && blockHeight != TX_UNCONFIRMED) {
					if (tx->GetTransactionType() != Transaction::CoinBase)
						needsUpdate = true;
					if (tx->GetTransactionType() == Transaction::RegisterAsset) {
						PayloadRegisterAsset *p = dynamic_cast<PayloadRegisterAsset *>(tx->GetPayload());
						if (p) payloads.push_back(p);
					}
				}

				tx->SetTimestamp(timestamp);
				tx->SetBlockHeight(blockHeight);

				if (ContainsTx(tx)) {
					for (size_t k = _transactions.size(); k > 0; k--) {
						if (_transactions[k - 1]->IsEqual(tx.get())) {
							_transactions.erase(_transactions.begin() + k - 1);
							InsertTx(tx);
						}
					}
					hashes.push_back(txHashes[i]);
					if (_invalidTx.Contains(tx) && tx->GetTransactionType() != Transaction::CoinBase) {
						needsUpdate = true;
					}
				} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
					_allTx.Remove(tx);
				}
			}

			if (!payloads.empty()) {
				for (size_t i = 0; i < payloads.size(); ++i)
					InstallAssets({payloads[i]->GetAsset()});
			}

			if (needsUpdate)
				changedBalance = UpdateBalanceInternal();

			Unlock();

			if (!hashes.empty()) txUpdated(hashes, blockHeight, timestamp);
			if (!payloads.empty()) {
				for (size_t i = 0; i < payloads.size(); ++i)
					assetRegistered(payloads[i]->GetAsset(), payloads[i]->GetAmount(), payloads[i]->GetController());
			}

			if (needsUpdate) {
				for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
					balanceChanged(it->first, it->second);
			}
		}

		TransactionPtr Wallet::TransactionForHash(const uint256 &txHash) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _allTx.Get(txHash);
		}

		bool Wallet::TransactionIsValid(const TransactionPtr &tx) {
			bool r = true;
			if (tx == nullptr || !tx->IsSigned())
				return false;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

			if (tx->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be invalid
				Lock();
				if (!_allTx.Contains(tx)) {
					for (size_t i = 0; r && i < tx->GetInputs().size(); ++i) {
						if (_spentOutputs.Contains(tx->GetInputs()[i]))
							r = false;
					}
				} else if (_invalidTx.Contains(tx)) {
					r = false;
				}
				Unlock();

				for (size_t i = 0; r && i < tx->GetInputs().size(); ++i) {
					TransactionPtr t = TransactionForHash(tx->GetInputs()[i].GetTransctionHash());
					if (t && !TransactionIsValid(t))
						r = false;
				}
			}

			return r;
		}

#if 0
		bool Wallet::TransactionIsPending(const TransactionPtr &transaction) {
			time_t now = time(NULL);
			uint32_t height;
			bool r = false;

			assert(transaction->IsSigned());

			Lock();
			height = _blockHeight;
			Unlock();


			if (transaction != nullptr &&
				transaction->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be postdated
				if (transaction->GetSize() > TX_MAX_SIZE) r = true; // check transaction size is under TX_MAX_SIZE

				for (size_t i = 0; !r && i < transaction->GetInputs().size(); i++) {
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX - 1) r = true; // check for replace-by-fee
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX &&
						transaction->GetLockTime() < TX_MAX_LOCK_HEIGHT &&
						transaction->GetLockTime() > height + 1)
						r = true; // future lockTime
					if (transaction->GetInputs()[i].GetSequence() < UINT32_MAX && transaction->GetLockTime() > now)
						r = true; // future lockTime
				}

				for (size_t i = 0; !r && i < transaction->GetOutputs().size(); i++) { // check that no outputs are dust
					if (transaction->GetOutputs()[i].GetAmount() < TX_MIN_OUTPUT_AMOUNT) r = true;
				}

				for (size_t i = 0;
					 !r && i < transaction->GetInputs().size(); i++) { // check if any inputs are known to be pending
					const TransactionPtr &t = TransactionForHash(transaction->GetInputs()[i].GetTransctionHash());
					if (t && TransactionIsPending(t)) r = true;
				}
			}

			return r;
		}

		bool Wallet::TransactionIsVerified(const TransactionPtr &transaction) {
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

		BigInt Wallet::AmountSentByTx(const TransactionPtr &tx) {
			BigInt amount(0);

			boost::mutex::scoped_lock scoped_lock(lock);
			for (size_t i = 0; tx && i < tx->GetInputs().size(); i++) {
				const TransactionPtr t = _allTx.Get(tx->GetInputs()[i].GetTransctionHash());
				uint32_t n = tx->GetInputs()[i].GetIndex();

				if (t && n < t->GetOutputs().size() &&
					_subAccount->ContainsAddress(t->GetOutputs()[n].GetAddress())) {
					amount += t->GetOutputs()[n].GetAmount();
				}
			}

			return amount;
		}

		Address Wallet::GetReceiveAddress() const {
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0];
		}

		size_t Wallet::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) {
			return _subAccount->GetAllAddresses(addr, start, count, containInternal);
		}

		Address Wallet::GetOwnerDepositAddress() const {
			if (Account::MultiSign == _subAccount->Parent()->GetSignType()) {
				return Address();
			}

			return Address(PrefixDeposit, _subAccount->OwnerPubKey());
		}

		Address Wallet::GetOwnerAddress() const {
			if (Account::MultiSign == _subAccount->Parent()->GetSignType()) {
				return Address();
			}

			return Address(PrefixStandard, _subAccount->OwnerPubKey());
		}

		bool Wallet::IsVoteDepositAddress(const Address &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->IsDepositAddress(addr);
		}

		bool Wallet::ContainsAddress(const Address &address) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _subAccount->ContainsAddress(address);
		}

		void Wallet::UpdateBalance() {
			boost::mutex::scoped_lock scopedLock(lock);
			UpdateBalanceInternal();
		}

		const std::string &Wallet::GetWalletID() const {
			return _walletID;
		}

		void Wallet::SetWalletID(const std::string &walletID) {
			boost::mutex::scoped_lock scopedLock(lock);
			_walletID = walletID;
		}

		void Wallet::SetBlockHeight(uint32_t height) {
			boost::mutex::scoped_lock scopedLock(lock);
			_blockHeight = height;
		}

		void Wallet::SignTransaction(const TransactionPtr &tx, const std::string &payPassword) {
			_subAccount->SignTransaction(tx, payPassword);
		}

		std::vector<TransactionPtr> Wallet::TxUnconfirmedBefore(uint32_t blockHeight) {
			boost::mutex::scoped_lock scopedLock(lock);
			size_t total, n = 0;
			std::vector<TransactionPtr> result;

			total = _transactions.size();
			while (n < total && _transactions[(total - n) - 1]->GetBlockHeight() >= blockHeight) n++;

			result.reserve(n);
			for (size_t i = 0; i < n; i++) {
				result.push_back(_transactions[(total - n) + i]);
			}

			return result;
		}

		void Wallet::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			size_t i, j, count;

			Lock();
			_blockHeight = blockHeight;
			count = i = _transactions.size();
			while (i > 0 && _transactions[i - 1]->GetBlockHeight() > blockHeight) i--;
			count -= i;

			std::vector<uint256> hashes;

			for (j = 0; j < count; j++) {
				_transactions[i + j]->SetBlockHeight(TX_UNCONFIRMED);
				hashes.push_back(_transactions[i + j]->GetHash());
			}

			if (count > 0) UpdateBalanceInternal();
			Unlock();

			if (count > 0) txUpdated(hashes, TX_UNCONFIRMED, 0);
		}

		const std::vector<std::string> &Wallet::GetListeningAddrs() const {
			return _listeningAddrs;
		}

		std::vector<Address> Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> Wallet::GetAllTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _transactions;
		}

		AssetPtr Wallet::GetAsset(const uint256 &assetID) const {
			boost::mutex::scoped_lock scopedLock(lock);
			if (!ContainsAsset(assetID)) {
				Log::warn("asset not found: {}", assetID.GetHex());
				return nullptr;
			}

			return _groupedAssets[assetID]->GetAsset();
		}

		nlohmann::json Wallet::GetAllAssets() const {
			boost::mutex::scoped_lock scopedLock(lock);
			nlohmann::json j;
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				j.push_back(it->first.GetHex());
			}
			return j;
		}

		bool Wallet::AssetNameExist(const std::string &name) const {
			boost::mutex::scoped_lock scopedLock(lock);
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it)
				if (it->second->GetAsset()->GetName() == name)
					return true;
			return false;
		}

		bool Wallet::ContainsAsset(const uint256 &assetID) const {
			return _groupedAssets.find(assetID) != _groupedAssets.end();
		}

		bool Wallet::ContainsTx(const TransactionPtr &tx) const {
			bool r = false;

			if (tx == nullptr)
				return r;

			// support register asset tx
			if (tx->GetTransactionType() == Transaction::RegisterAsset) {
				return true;
			}

			size_t outCount = tx->GetOutputs().size();

			for (size_t i = 0; !r && i < outCount; i++) {
				if (_subAccount->ContainsAddress(tx->GetOutputs()[i].GetAddress())) {
					r = true;
				}
			}

			for (size_t i = 0; !r && i < tx->GetInputs().size(); i++) {
				const TransactionPtr t = _allTx.Get(tx->GetInputs()[i].GetTransctionHash());
				uint32_t n = tx->GetInputs()[i].GetIndex();

				if (t == nullptr || n >= t->GetOutputs().size()) {
					continue;
				}

				if (_subAccount->ContainsAddress(t->GetOutputs()[n].GetAddress()))
					r = true;
			}

			//for listening addresses
			for (size_t i = 0; i < outCount; ++i) {
				if (std::find(_listeningAddrs.begin(), _listeningAddrs.end(),
							  tx->GetOutputs()[i].GetAddress().String()) != _listeningAddrs.end())
					r = true;
			}

			return r;
		}

		void Wallet::InsertTx(const TransactionPtr &tx) {
			size_t i = _transactions.size();

			_transactions.resize(i + 1);

			while (i > 0 && TxCompare(_transactions[i - 1], tx) > 0) {
				_transactions[i] = _transactions[i - 1];
				i--;
			}

			_transactions[i] = tx;
		}

		int Wallet::TxCompare(const TransactionPtr &tx1, const TransactionPtr &tx2) const {
			size_t i = -1, j = -1;

			if (TxIsAscending(tx1, tx2))
				return 1;
			if (TxIsAscending(tx2, tx1))
				return -1;
			if ((i = _subAccount->InternalChainIndex(tx1)) != -1)
				j = _subAccount->InternalChainIndex(tx2);
			if (j == -1 && (i = _subAccount->ExternalChainIndex(tx1)) != -1)
				j = _subAccount->ExternalChainIndex(tx2);
			if (i != -1 && j != -1 && i != j)
				return (i > j) ? 1 : -1;
			return 0;
		}

		bool Wallet::TxIsAscending(const TransactionPtr &tx1, const TransactionPtr &tx2) const {
			if (! tx1 || ! tx2)
				return false;

			if (tx1->GetBlockHeight() > tx2->GetBlockHeight()) return 1;
			if (tx1->GetBlockHeight() < tx2->GetBlockHeight()) return 0;

			for (size_t i = 0; i < tx1->GetInputs().size(); i++) {
				if (tx1->GetInputs()[i].GetTransctionHash() == tx2->GetHash()) return 1;
			}

			for (size_t i = 0; i < tx2->GetInputs().size(); i++) {
				if (tx2->GetInputs()[i].GetTransctionHash() == tx1->GetHash()) return 0;
			}

			for (size_t i = 0; i < tx1->GetInputs().size(); i++) {
				if (TxIsAscending(_allTx.Get(tx1->GetInputs()[i].GetTransctionHash()), tx2)) return 1;
			}

			return 0;
		}

		std::vector<UTXO> Wallet::GetUTXO(const uint256 &assetID) const {
			if (!ContainsAsset(assetID)) {
				Log::error("asset not found: {}", assetID.GetHex());
				return {};
			}

			return _groupedAssets[assetID]->GetUTXOs();
		}

		bool Wallet::IsAssetUnique(const std::vector<TransactionOutput> &outputs) const {
			for (size_t i = 1; i < outputs.size(); ++i) {
				if (outputs[0].GetAssetID() != outputs[i].GetAssetID())
					return false;
			}

			return true;
		}

		std::map<uint256, BigInt> Wallet::UpdateBalanceInternal() {
			std::map<uint256, BigInt> changedBalance;
			int isInvalid;
			size_t i, j;

			_spentOutputs.Clear();
			_spendingOutputs.Clear();
			_invalidTx.Clear();
			_subAccount->ClearUsedAddresses();

			GroupedAssetMap::iterator it;
			for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				it->second->CleanBalance();
			}

			for (i = 0; i < _transactions.size(); i++) {
				const TransactionPtr &tx = _transactions[i];

				// check if any inputs are invalid or already spent
				if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
					for (j = 0, isInvalid = 0; !isInvalid && j < tx->GetInputs().size(); j++) {
						if (_spentOutputs.Contains(tx->GetInputs()[j].GetTransctionHash(), tx->GetInputs()[j].GetIndex()) ||
							_invalidTx.Contains(tx->GetInputs()[j].GetTransctionHash()))
							isInvalid = 1;
					}

					if (isInvalid) {
						_invalidTx.Insert(tx);
					} else {
						// add inputs to spent output set
						for (j = 0; j < tx->GetInputs().size(); j++) {
							_spendingOutputs.AddByTxInput(tx->GetInputs()[j], 0);
						}
					}
					continue;
				}

				// add inputs to spent output set
				for (j = 0; j < tx->GetInputs().size(); j++) {
					_spentOutputs.AddByTxInput(tx->GetInputs()[j], 0);
				}

				const std::vector<TransactionOutput> &outputs = tx->GetOutputs();
				for (j = 0; j < outputs.size(); j++) {
					Address addr = outputs[j].GetAddress();
					if (_subAccount->ContainsAddress(addr)) {
						_subAccount->AddUsedAddrs(addr);
						uint256 assetID = outputs[j].GetAssetID();
						if (ContainsAsset(assetID)) {
							_groupedAssets[assetID]->AddUTXO(UTXO(tx->GetHash(), (uint16_t) j, outputs[j].GetAmount()));
						}
					}
				}
			}

			for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				changedBalance[it->first] = it->second->UpdateBalance();
			}

			return changedBalance;
		}

		void Wallet::InstallAssets(const std::vector<AssetPtr> &assets) {
			for (size_t i = 0; i < assets.size(); ++i) {
				if (!ContainsAsset(assets[i]->GetHash())) {
					_groupedAssets[assets[i]->GetHash()] = GroupedAssetPtr(new GroupedAsset(this, assets[i]));
				} else {
					Log::debug("asset {} already exist", assets[i]->GetHash().GetHex());
				}
			}
		}

		void Wallet::InstallDefaultAsset() {
			AssetPtr asset(new Asset());
			_groupedAssets[asset->GetHash()] = GroupedAssetPtr(new GroupedAsset(this, asset));
			assetRegistered(asset, 0, uint168());
		}

		void Wallet::balanceChanged(const uint256 &asset, const BigInt &balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(asset, balance);
			}
		}

		void Wallet::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void Wallet::txUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, uint32_t timestamp) {
			if (!_listener.expired()) {
				// Invoke the callback for each of txHashes.
				for (size_t i = 0; i < txHashes.size(); i++) {
					_listener.lock()->onTxUpdated(txHashes[i], blockHeight, timestamp);
				}
			}
		}

		void Wallet::txDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onTxDeleted(txHash, notifyUser, recommendRescan);
			}
		}

		void Wallet::assetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			if (!_listener.expired()) {
				_listener.lock()->onAssetRegistered(asset, amount, controller);
			}
		}

	}
}