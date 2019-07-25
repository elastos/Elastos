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
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/TransactionInput.h>
#include <SDK/Plugin/Transaction/Payload/RegisterAsset.h>
#include <SDK/Wallet/UTXO.h>

#include <Interface/ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		Wallet::Wallet(uint32_t lastBlockHeight,
					   const std::vector<AssetPtr> &assetArray,
					   const std::vector<TransactionPtr> &txns,
					   const UTXOArray &cbUTXOs,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Wallet::Listener> &listener) :
				_blockHeight(lastBlockHeight),
				_feePerKb(DEFAULT_FEE_PER_KB),
				_subAccount(subAccount) {

			_listener = boost::weak_ptr<Listener>(listener);

			_subAccount->Init(txns, this);

			if (assetArray.empty()) {
				InstallDefaultAsset();
			} else {
				InstallAssets(assetArray);
			}

			if (!txns.empty() && !ContainsTx(txns[0])) { // verify _transactions match master pubKey
				std::string hash = txns[0]->GetHash().GetHex();
				ErrorChecker::ThrowLogicException(Error::WalletNotContainTx, "Wallet do not contain tx = " + hash);
			}

			bool stripped = false, movedToCoinbase = false;
			std::set<uint256> spentHashes;
			for (size_t i = 0; i < txns.size(); ++i) {
				if (txns[i]->IsCoinBase()) {
					movedToCoinbase = true;
					const OutputArray &outputs = txns[i]->GetOutputs();
					for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
						if (_subAccount->ContainsAddress((*o)->Addr())) {
							UTXOPtr cb(new UTXO(txns[i]->GetHash(), (*o)->FixedIndex(), txns[i]->GetTimestamp(),
												txns[i]->GetBlockHeight(), *o));
							_coinBaseUTXOs.push_back(cb);
							break;
						}
					}
				} else {
					txns[i]->IsRegistered() = true;
					if (StripTransaction(txns[i]))
						stripped = true;

					if (/*!txns[i]->IsSigned() || */_allTx.Contains(txns[i]))
						continue;

					for (InputArray::iterator in = txns[i]->GetInputs().begin(); in != txns[i]->GetInputs().end(); ++in)
						spentHashes.insert((*in)->TxHash());

					_allTx.Insert(txns[i]);
					InsertTx(txns[i]);

					if (txns[i]->GetBlockHeight() != TX_UNCONFIRMED)
						BalanceAfterUpdatedTx(txns[i]);
				}
			}

			_coinBaseUTXOs.insert(_coinBaseUTXOs.end(), cbUTXOs.begin(), cbUTXOs.end());

			for (std::set<uint256>::iterator it = spentHashes.begin(); it != spentHashes.end(); ++it) {
				for (UTXOArray::iterator cb = _coinBaseUTXOs.begin(); cb != _coinBaseUTXOs.end(); ++cb) {
					if ((*it) == (*cb)->Hash()) {
						(*cb)->SetSpent(true);
						break;
					}
				}
			}

			for (UTXOArray::iterator o = _coinBaseUTXOs.begin(); o != _coinBaseUTXOs.end(); ++o) {
				if (!(*o)->Spent())
					_groupedAssets[(*o)->Output()->AssetID()]->AddCoinBaseUTXO((*o));
			}

			if (movedToCoinbase)
				coinBaseUpdatedAll(_coinBaseUTXOs);

			if (stripped)
				txUpdatedAll(_transactions);
		}

		Wallet::~Wallet() {
		}

		void Wallet::InitListeningAddresses(const std::vector<std::string> &addrs) {
			boost::mutex::scoped_lock scopedLock(lock);
			_listeningAddrs = addrs;
		}

		std::vector<UTXOPtr> Wallet::GetAllUTXO(const std::string &address) const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<UTXOPtr> result;

			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				std::vector<UTXOPtr> utxos = it->second->GetUTXOs(address);
				result.insert(result.end(), utxos.begin(), utxos.end());
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

		BigInt Wallet::GetBalanceWithAddress(const uint256 &assetID, const std::string &addr,
											 GroupedAsset::BalanceType type) const {
			boost::mutex::scoped_lock scopedLock(lock);

			BigInt balance = 0;
			std::vector<UTXOPtr> utxos = GetUTXO(assetID, addr);

			for (size_t i = 0; i < utxos.size(); ++i) {
				const TransactionPtr tx = _allTx.Get(utxos[i]->Hash());
				if (tx == nullptr ||
					(type == GroupedAsset::Default &&
						tx->GetOutputs()[utxos[i]->Index()]->GetType() != TransactionOutput::Type::Default) ||
					(type == GroupedAsset::Voted &&
						tx->GetOutputs()[utxos[i]->Index()]->GetType() != TransactionOutput::Type::VoteOutput)) {
					continue;
				}

				balance += tx->GetOutputs()[utxos[i]->Index()]->Amount();
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

		TransactionPtr Wallet::Consolidate(const std::string &memo, const uint256 &assetID, bool userVotedUTXO) {
			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->Consolidate(memo, userVotedUTXO);

			if (assetID != Asset::GetELAAssetID())
				_groupedAssets[Asset::GetELAAssetID()]->AddFeeForTx(tx, false);

			return tx;
		}

		TransactionPtr Wallet::CreateTransaction(const Address &fromAddress,
												 const std::vector<OutputPtr> &outputs,
												 const std::string &memo,
												 bool useVotedUTXO, bool autoReduceOutputAmount) {

			ErrorChecker::CheckParam(!IsAssetUnique(outputs), Error::InvalidAsset, "asset is not unique in outputs");

			uint256 assetID = outputs.front()->AssetID();

			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx;
			if (fromAddress.Valid() && _subAccount->IsDepositAddress(fromAddress))
				tx = _groupedAssets[assetID]->CreateRetrieveDepositTx(outputs, fromAddress, memo);
			else
				tx = _groupedAssets[assetID]->CreateTxForOutputs(outputs, fromAddress, memo, useVotedUTXO,
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
			UTXOPtr cb = nullptr;

			bool IsReceiveTx = IsReceiveTransaction(tx);
			if (tx != nullptr && (IsReceiveTx || (!IsReceiveTx && tx->IsSigned()))) {
				Lock();
				if (!tx->IsCoinBase() && !_allTx.Contains(tx)) {
					if (ContainsTx(tx)) {
						// TODO: verify signatures when possible
						// TODO: handle tx replacement with input sequence numbers
						//       (for now, replacements appear invalid until confirmation)
						_allTx.Insert(tx);
						InsertTx(tx);
						AddSpendingUTXO(tx->GetInputs());
						wasAdded = true;
					} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
						// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
						if (tx->GetBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(tx);
						r = false;
						// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
					}
				} else if (tx->IsCoinBase() && !CoinBaseContains(tx->GetHash())) {
					cb = RegisterCoinBaseTx(tx);
				}
				Unlock();
			} else {
				r = false;
			}

			if (wasAdded) {
				UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				txAdded(tx);
			} else if (cb) {
				coinBaseTxAdded(cb);
			}

			return r;
		}

		void Wallet::RemoveTransaction(const uint256 &txHash) {
			bool notifyUser = false, recommendRescan = false;
			std::vector<uint256> hashes;

			assert(txHash != 0);

			Lock();
			const TransactionPtr tx = _allTx.Get(txHash);

			if (tx) {
				for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
					const TransactionPtr &t = _transactions[i - 1];
					if (t->GetBlockHeight() < tx->GetBlockHeight()) break;
					if (tx->IsEqual(t.get())) continue;

					for (size_t j = 0; j < t->GetInputs().size(); j++) {
						if (t->GetInputs()[j]->TxHash() != txHash) continue;
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
						if (_transactions[i - 1]->IsEqual(tx.get())) {
							_transactions.erase(_transactions.begin() + i - 1);
							break;
						}
					}

					BalanceAfterRemoveTx(tx);
					Unlock();

					// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
					if (AmountSentByTx(tx) > 0 && TransactionIsValid(tx)) {
						recommendRescan = notifyUser = true;

						for (size_t i = 0;
							 i < tx->GetInputs().size(); i++) { // only recommend a rescan if all inputs are confirmed
							TransactionPtr t = TransactionForHash(tx->GetInputs()[i]->TxHash());
							if (t && t->GetBlockHeight() != TX_UNCONFIRMED) continue;
							recommendRescan = false;
							break;
						}
					}

					txDeleted(tx->GetHash(), notifyUser, recommendRescan);
				}
			} else {
				Unlock();
			}
		}

		void Wallet::UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			std::vector<uint256> hashes, cbHashes, spentCoinBase;
			std::map<uint256, BigInt> changedBalance;
			std::vector<RegisterAsset *> payloads;
			UTXOPtr cb;
			size_t i;

			Lock();
			if (blockHeight != TX_UNCONFIRMED && blockHeight > _blockHeight)
				_blockHeight = blockHeight;

			for (i = 0; i < txHashes.size(); i++) {
				TransactionPtr tx = _allTx.Get(txHashes[i]);
				if (tx) {
					if (tx->GetBlockHeight() == blockHeight && tx->GetTimestamp() == timestamp)
						continue;

					if (tx->GetBlockHeight() == TX_UNCONFIRMED && blockHeight != TX_UNCONFIRMED) {
						if (tx->GetTransactionType() == Transaction::registerAsset) {
							RegisterAsset *p = dynamic_cast<RegisterAsset *>(tx->GetPayload());
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
						RemoveSpendingUTXO(tx->GetInputs());
						GetSpentCoinbase(tx->GetInputs(), spentCoinBase);
						changedBalance = BalanceAfterUpdatedTx(tx);
					} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
						_allTx.Remove(tx);
					}
				} else if ((cb = CoinBaseForHashInternal(txHashes[i])) != nullptr) {
					if (cb->BlockHeight() == blockHeight && cb->Timestamp() == timestamp)
						continue;

					cb->SetTimestamp(timestamp);
					cb->SetBlockHeight(blockHeight);
					_groupedAssets[cb->Output()->AssetID()]->AddCoinBaseUTXO(cb);
					cbHashes.push_back(txHashes[i]);
				}
			}

			if (!payloads.empty()) {
				for (i = 0; i < payloads.size(); ++i)
					InstallAssets({payloads[i]->GetAsset()});
			}

			Unlock();

			if (!hashes.empty())
				txUpdated(hashes, blockHeight, timestamp);

			if (!cbHashes.empty())
				coinBaseTxUpdated(cbHashes, blockHeight, timestamp);

			if (!spentCoinBase.empty()) {
				coinBaseSpent(spentCoinBase);
			}

			if (!payloads.empty()) {
				for (i = 0; i < payloads.size(); ++i)
					assetRegistered(payloads[i]->GetAsset(), payloads[i]->GetAmount(), payloads[i]->GetController());
			}

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
		}

		TransactionPtr Wallet::TransactionForHash(const uint256 &txHash) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _allTx.Get(txHash);
		}

		UTXOPtr Wallet::CoinBaseTxForHash(const uint256 &txHash) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return CoinBaseForHashInternal(txHash);
		}

		bool Wallet::TransactionIsValid(const TransactionPtr &tx) {
			bool r = true;
			if (tx == nullptr || !tx->IsSigned())
				return false;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

			if (tx->GetBlockHeight() == TX_UNCONFIRMED) { // only unconfirmed _transactions can be invalid
//				Lock();
//				if (!_allTx.Contains(tx)) {
//					for (size_t i = 0; r && i < tx->GetInputs().size(); ++i) {
//						if (_spentOutputs.Contains(tx->GetInputs()[i]))
//							r = false;
//					}
//				} else if (_invalidTx.Contains(tx)) {
//					r = false;
//				}
//				Unlock();

				for (size_t i = 0; r && i < tx->GetInputs().size(); ++i) {
					TransactionPtr t = TransactionForHash(tx->GetInputs()[i]->TxHash());
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
					if (transaction->GetOutputs()[i].Amount() < TX_MIN_OUTPUT_AMOUNT) r = true;
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

			boost::mutex::scoped_lock scopedLock(lock);
			for (size_t i = 0; tx && i < tx->GetInputs().size(); i++) {
				TransactionPtr t = _allTx.Get(tx->GetInputs()[i]->TxHash());
				uint32_t n = tx->GetInputs()[i]->Index();

				if (t && n < t->GetOutputs().size() &&
					_subAccount->ContainsAddress(t->GetOutputs()[n]->Addr())) {
					amount += t->GetOutputs()[n]->Amount();
				}
			}

			return amount;
		}

		bool Wallet::IsReceiveTransaction(const TransactionPtr &tx) const {
			boost::mutex::scoped_lock scopedLock(lock);
			bool status = true;
			for (InputArray::iterator in = tx->GetInputs().begin(); in != tx->GetInputs().end(); ++in) {
				UTXOPtr cb;
				TransactionPtr t = _allTx.Get((*in)->TxHash());
				if (t) {
					OutputPtr output = t->OutputOfIndex((*in)->Index());
					if (output && _subAccount->ContainsAddress(output->Addr())) {
						status = false;
						break;
					}
				} else if ((cb = CoinBaseForHashInternal((*in)->TxHash())) != nullptr) {
					status = false;
					break;
				}
			}

			return status;
		}

		bool Wallet::StripTransaction(const TransactionPtr &tx) const {
			if (IsReceiveTransaction(tx) && tx->GetOutputs().size() > 2) {
				boost::mutex::scoped_lock scopedLock(lock);
				std::vector<OutputPtr> newOutputs;
				const std::vector<OutputPtr> &outputs = tx->GetOutputs();
				for (size_t i = 0; i < outputs.size(); ++i) {
					if (_subAccount->ContainsAddress(outputs[i]->Addr())) {
						outputs[i]->SetFixedIndex((uint16_t)i);
						newOutputs.push_back(outputs[i]);
					}
				}
				tx->SetOutputs(newOutputs);
				return true;
			}
			return false;
		}

		Address Wallet::GetReceiveAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<Address> addr = _subAccount->UnusedAddresses(1, 0);
			return addr[0];
		}

		size_t Wallet::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) {
			boost::mutex::scoped_lock scopedLock(lock);

			return _subAccount->GetAllAddresses(addr, start, count, containInternal);
		}

		Address Wallet::GetOwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return Address(PrefixDeposit, *_subAccount->OwnerPubKey());
		}

		Address Wallet::GetCROwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return Address(PrefixDeposit, _subAccount->DIDPubKey());
		}

		Address Wallet::GetOwnerAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return Address(PrefixStandard, *_subAccount->OwnerPubKey());
		}

		std::vector<Address> Wallet::GetAllSpecialAddresses() const {
			std::vector<Address> result;
			boost::mutex::scoped_lock scopedLock(lock);
			// Owner address
			result.push_back(Address(PrefixStandard, *_subAccount->OwnerPubKey()));
			// Owner deposit address
			result.push_back(Address(PrefixDeposit, *_subAccount->OwnerPubKey()));
			// CR Owner deposit address
			result.push_back(Address(PrefixStandard, _subAccount->DIDPubKey()));

			return result;
		}

		bytes_ptr Wallet::GetOwnerPublilcKey() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->OwnerPubKey();
		}

		bool Wallet::IsVoteDepositAddress(const Address &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->IsDepositAddress(addr);
		}

		bool Wallet::ContainsAddress(const Address &address) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _subAccount->ContainsAddress(address);
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

			for (j = count; j > 0; --j) {
				if (_transactions[i + j - 1]->GetBlockHeight() != TX_UNCONFIRMED) {
					_transactions[i + j - 1]->SetBlockHeight(TX_UNCONFIRMED);
					hashes.push_back(_transactions[i + j - 1]->GetHash());
					BalanceAfterUpdatedTx(_transactions[i + j - 1]);
				}
			}

			Unlock();

			if (count > 0) txUpdated(hashes, TX_UNCONFIRMED, 0);
		}

		const std::vector<std::string> &Wallet::GetListeningAddrs() const {
			return _listeningAddrs;
		}

		std::vector<Address> Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> Wallet::GetAllTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _transactions;
		}

		std::vector<UTXOPtr> Wallet::GetAllCoinBaseTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _coinBaseUTXOs;
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
			if (tx->GetTransactionType() == Transaction::registerAsset) {
				return true;
			}

			const std::vector<OutputPtr> &outputs = tx->GetOutputs();
			std::vector<OutputPtr>::const_iterator it;
			for (it = outputs.cbegin(); !r && it != outputs.cend(); ++it) {
				if (_subAccount->ContainsAddress((*it)->Addr()))
					r = true;

				if (std::find(_listeningAddrs.begin(), _listeningAddrs.end(),
							  (*it)->Addr().String()) != _listeningAddrs.end())
					r = true;
			}

			for (size_t i = 0; !r && i < tx->GetInputs().size(); i++) {
				const TransactionPtr t = _allTx.Get(tx->GetInputs()[i]->TxHash());
				if (!t) continue;

				uint32_t n = tx->GetInputs()[i]->Index();
				OutputPtr output = t->OutputOfIndex(tx->GetInputs()[i]->Index());
				if (!output) continue;

				if (_subAccount->ContainsAddress(output->Addr()))
					r = true;
			}

			return r;
		}

		bool Wallet::CoinBaseContains(const uint256 &txHash) const {
			return nullptr != CoinBaseForHashInternal(txHash);
		}

		UTXOPtr Wallet::CoinBaseForHashInternal(const uint256 &txHash) const {
			for (size_t i = 0; i < _coinBaseUTXOs.size(); ++i) {
				if (_coinBaseUTXOs[i]->Hash() == txHash) {
					return _coinBaseUTXOs[i];
				}
			}

			return nullptr;
		}

		UTXOPtr Wallet::RegisterCoinBaseTx(const TransactionPtr &tx) {
			const OutputArray &outputs = tx->GetOutputs();
			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
				if (_subAccount->ContainsAddress((*o)->Addr())) {
					UTXOPtr cb(new UTXO(tx->GetHash(), (*o)->FixedIndex(), tx->GetTimestamp(), tx->GetBlockHeight(), (*o)));
					_coinBaseUTXOs.push_back(cb);
					return cb;
				}
			}

			return nullptr;
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
				if (tx1->GetInputs()[i]->TxHash() == tx2->GetHash()) return 1;
			}

			for (size_t i = 0; i < tx2->GetInputs().size(); i++) {
				if (tx2->GetInputs()[i]->TxHash() == tx1->GetHash()) return 0;
			}

			for (size_t i = 0; i < tx1->GetInputs().size(); i++) {
				if (TxIsAscending(_allTx.Get(tx1->GetInputs()[i]->TxHash()), tx2)) return 1;
			}

			return 0;
		}

		std::vector<UTXOPtr> Wallet::GetUTXO(const uint256 &assetID, const std::string &addr) const {
			if (!ContainsAsset(assetID)) {
				Log::error("asset not found: {}", assetID.GetHex());
				return {};
			}

			return _groupedAssets[assetID]->GetUTXOs(addr);
		}

		bool Wallet::IsAssetUnique(const std::vector<OutputPtr> &outputs) const {
			for (size_t i = 1; i < outputs.size(); ++i) {
				if (outputs[0]->AssetID() != outputs[i]->AssetID())
					return false;
			}

			return true;
		}

		std::map<uint256, BigInt> Wallet::BalanceAfterUpdatedTx(const TransactionPtr &tx) {
			GroupedAssetMap::iterator it;
			std::map<uint256, BigInt> changedBalance;
			if (tx->GetBlockHeight() != TX_UNCONFIRMED) {
				for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
					if (it->second->RemoveSpentUTXO(tx->GetInputs())) {
						changedBalance[it->first] = 0;
					}
				}

				const OutputArray &outputs = tx->GetOutputs();
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (_subAccount->ContainsAddress((*o)->Addr())) {
						const uint256 &asset = (*o)->AssetID();
						if (ContainsAsset(asset)) {
							uint16_t n = (*o)->FixedIndex();
							UTXOPtr utxo(new UTXO(tx->GetHash(), n, tx->GetTimestamp(), tx->GetBlockHeight(), (*o)));
							_groupedAssets[asset]->AddUTXO(utxo);
							changedBalance[asset] = 0;
						}
					}
				}
			} else {
				const OutputArray &outputs = tx->GetOutputs();
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (_subAccount->ContainsAddress((*o)->Addr()) && ContainsAsset((*o)->AssetID())) {
						if (_groupedAssets[(*o)->AssetID()]->RemoveSpentUTXO(tx->GetHash(), (*o)->FixedIndex())) {
							changedBalance[(*o)->AssetID()] = 0;
						}
					}
				}

				const InputArray &inputs = tx->GetInputs();
				for (InputArray::const_iterator in = inputs.cbegin(); in != inputs.cend(); ++in) {
					const TransactionPtr txInput = _allTx.Get((*in)->TxHash());
					UTXOPtr cb;
					if (txInput && txInput->GetBlockHeight() != TX_UNCONFIRMED) {
						for (OutputArray::const_iterator o = txInput->GetOutputs().cbegin(); o != txInput->GetOutputs().cend(); ++o) {
							if (_subAccount->ContainsAddress((*o)->Addr()) && ContainsAsset((*o)->AssetID())) {
								UTXOPtr utxo(new UTXO(txInput->GetHash(), (*o)->FixedIndex(), txInput->GetTimestamp(), txInput->GetBlockHeight(), (*o)));
								_groupedAssets[(*o)->AssetID()]->AddUTXO(utxo);
								changedBalance[(*o)->AssetID()] = 0;
							}
						}
					} else if ((cb = CoinBaseForHashInternal((*in)->TxHash())) != nullptr) {
						// TODO BUG update spent status to database
						cb->SetSpent(false);
						if (ContainsAsset(cb->Output()->AssetID())) {
							_groupedAssets[cb->Output()->AssetID()]->AddCoinBaseUTXO(cb);
							changedBalance[cb->Output()->AssetID()] = 0;
						}
					}
				}
			}

			for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				changedBalance[it->first] = it->second->GetBalance();
			}

			return changedBalance;
		}

		void Wallet::BalanceAfterRemoveTx(const TransactionPtr &tx) {
			if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
				RemoveSpendingUTXO(tx->GetInputs());
			} else {
				// TODO: consider rollback later
			}
		}

		void Wallet::RemoveSpendingUTXO(const InputArray &inputs) {
			for (InputArray::const_iterator input = inputs.cbegin(); input != inputs.cend(); ++input) {
				for (UTXOArray::iterator o = _spendingOutputs.begin(); o != _spendingOutputs.end(); ++o) {
					if ((*o)->Equal(*input)) {
						_spendingOutputs.erase(o);
						break;
					}
				}
			}
		}

		void Wallet::AddSpendingUTXO(const InputArray &inputs) {
			for (InputArray::const_iterator input = inputs.cbegin(); input != inputs.cend(); ++input) {
				for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
					UTXOPtr cb = it->second->FindUTXO(*input);
					if (cb) {
						_spendingOutputs.push_back(cb);
						break;
					}
				}
			}
		}

		void Wallet::UpdateLockedBalance() {
			std::map<uint256, BigInt> changedBalance;

			lock.lock();
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				if (it->second->UpdateLockedBalance()) {
					changedBalance[it->first] = it->second->GetBalance();
				}
			}
			lock.unlock();

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
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

		bool Wallet::IsUTXOSpending(const UTXOPtr &utxo) const {
			for (size_t i = 0; i < _spendingOutputs.size(); ++i) {
				if (*utxo == *_spendingOutputs[i]) {
					return true;
				}
			}

			return false;
		}

		void Wallet::AddSpendingUTXO(const InputPtr &input) {
			UTXOPtr txo(new UTXO(*input));
			_spendingOutputs.push_back(txo);
		}

		void Wallet::GetSpentCoinbase(const InputArray &inputs, std::vector<uint256> &coinbase) const {
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				it->second->GetSpentCoinbase(inputs, coinbase);
			}
		}

		void Wallet::balanceChanged(const uint256 &asset, const BigInt &balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(asset, balance);
			}
		}

		void Wallet::coinBaseTxAdded(const UTXOPtr &cb) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseTxAdded(cb);
			}
		}

		void Wallet::coinBaseUpdatedAll(const UTXOArray &cbs) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseUpdatedAll(cbs);
			}
		}

		void Wallet::coinBaseTxUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseTxUpdated(txHashes, blockHeight, timestamp);
			}
		}

		void Wallet::coinBaseSpent(const std::vector<uint256> &spentHashes) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseSpent(spentHashes);
			}
		}

		void Wallet::coinBaseDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinBaseTxDeleted(txHash, notifyUser, recommendRescan);
			}
		}

		void Wallet::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onTxAdded(tx);
			}
		}

		void Wallet::txUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			if (!_listener.expired()) {
				_listener.lock()->onTxUpdated(txHashes, blockHeight, timestamp);
			}
		}

		void Wallet::txDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onTxDeleted(txHash, notifyUser, recommendRescan);
			}
		}

		void Wallet::txUpdatedAll(const std::vector<TransactionPtr> &txns) {
			if (!_listener.expired()) {
				_listener.lock()->onTxUpdatedAll(txns);
			}
		}

		void Wallet::assetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			if (!_listener.expired()) {
				_listener.lock()->onAssetRegistered(asset, amount, controller);
			}
		}

	}
}