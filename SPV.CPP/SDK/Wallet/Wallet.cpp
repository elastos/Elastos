// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "GroupedAsset.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Key.h>
#include <WalletCore/Address.h>
#include <WalletCore/HDKeychain.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Payload/RegisterAsset.h>
#include <Plugin/Registry.h>
#include <Wallet/UTXO.h>

#include <ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		Wallet::Wallet(uint32_t lastBlockHeight,
					   const std::string &walletID,
					   const std::string &chainID,
					   const std::vector<AssetPtr> &assetArray,
					   const std::vector<TransactionPtr> &txns,
					   const std::vector<TransactionPtr> &txCoinbase,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Wallet::Listener> &listener) :
			_walletID(walletID + ":" + chainID),
			_chainID(chainID),
			_blockHeight(lastBlockHeight),
			_feePerKb(DEFAULT_FEE_PER_KB),
			_subAccount(subAccount) {

			_listener = boost::weak_ptr<Listener>(listener);

			if (assetArray.empty()) {
				InstallDefaultAsset();
			} else {
				InstallAssets(assetArray);
			}

			_subAccount->Init();

			for (const TransactionPtr &tx : txCoinbase) {
				if (!_allTx.Insert(tx))
					continue;

				InsertTx(tx);
				BalanceAfterUpdatedTx(tx);
			}

			bool needUpdate = false, movedToCoinbase = false;
			for (size_t i = 0; i < txns.size(); ++i) {
				if (ContainsTx(txns[i])) {
					txns[i]->IsRegistered() = true;
					if (StripTransaction(txns[i])) {
						SPVLOG_INFO("{} strip tx: {}, h: {}, t: {}", _walletID, txns[i]->GetHash().GetHex(),
									txns[i]->GetBlockHeight(), txns[i]->GetTimestamp());
						needUpdate = true;
					}

					if (!_allTx.Insert(txns[i]))
						continue;

					if (txns[i]->IsCoinBase())
						movedToCoinbase = true;

					InsertTx(txns[i]);
					BalanceAfterUpdatedTx(txns[i]);

					if (txns[i]->GetBlockHeight() == TX_UNCONFIRMED)
						AddSpendingUTXO(txns[i]->GetInputs());
				} else {
					Log::error("Contain tx not belongs to current wallet");
					needUpdate = true;
				}
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
				_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
			}

			if (movedToCoinbase) {
				SPVLOG_INFO("{} mv coinbase tx to single table", _walletID);
				coinbaseTxMove(_coinbaseTransactions);
			}

			if (needUpdate) {
				SPVLOG_INFO("{} contain not striped tx, update all tx", _walletID);
				txUpdatedAll(_transactions);
			}
			SPVLOG_DEBUG("{} balance info {}", _walletID, GetBalanceInfo().dump(4));
		}

		Wallet::~Wallet() {
		}

		std::vector<UTXOPtr> Wallet::GetAllUTXO(const std::string &address) const {
			boost::mutex::scoped_lock scopedLock(lock);
			UTXOArray result;

			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				UTXOArray utxos = it->second->GetUTXOs(address);
				result.insert(result.end(), utxos.begin(), utxos.end());
			}

			return result;
		}

		UTXOArray Wallet::GetVoteUTXO() const {
			boost::mutex::scoped_lock scopedLock(lock);
			UTXOArray result;

			std::for_each(_groupedAssets.begin(), _groupedAssets.end(),
						  [&result](GroupedAssetMap::reference &asset) {
							  const UTXOSet &utxos = asset.second->GetVoteUTXO();
							  result.insert(result.end(), utxos.begin(), utxos.end());
						  });

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

		BigInt Wallet::GetBalanceWithAddress(const uint256 &assetID, const std::string &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);

			BigInt balance = 0;
			std::vector<UTXOPtr> utxos = GetUTXO(assetID, addr);

			for (size_t i = 0; i < utxos.size(); ++i) {
				balance += utxos[i]->Output()->Amount();
			}

			return balance;
		}

		BigInt Wallet::GetBalance(const uint256 &assetID) const {
			ErrorChecker::CheckParam(!ContainsAsset(assetID), Error::InvalidAsset, "asset not found");

			boost::mutex::scoped_lock scoped_lock(lock);

			return _groupedAssets[assetID]->GetBalance();
		}

		uint64_t Wallet::GetFeePerKb() const {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _feePerKb;
		}

		void Wallet::SetFeePerKb(uint64_t fee) {
			boost::mutex::scoped_lock scoped_lock(lock);
			_feePerKb = fee;
		}

		// only support asset of ELA
		TransactionPtr Wallet::Vote(const VoteContent &voteContent, const std::string &memo, bool max,
		                            VoteContentArray &dropedVotes) {
			return _groupedAssets[Asset::GetELAAssetID()]->Vote(voteContent, memo, max, dropedVotes);
		}

		TransactionPtr Wallet::Consolidate(const std::string &memo, const uint256 &assetID) {
			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->Consolidate(memo);

			if (assetID != Asset::GetELAAssetID())
				_groupedAssets[Asset::GetELAAssetID()]->AddFeeForTx(tx);

			return tx;
		}

		TransactionPtr Wallet::CreateRetrieveTransaction(uint8_t type, const PayloadPtr &payload, const BigInt &amount,
														 const AddressPtr &fromAddress, const std::string &memo) {
			std::string memoFixed;

			if (!memo.empty())
				memoFixed = "type:text,msg:" + memo;

			TransactionPtr tx = _groupedAssets[Asset::GetELAAssetID()]->CreateRetrieveDepositTx(type, payload, amount, fromAddress, memoFixed);
			tx->SetVersion(Transaction::TxVersion::V09);

			tx->FixIndex();
			return tx;
		}

		TransactionPtr Wallet::CreateTransaction(uint8_t type,
												 const PayloadPtr &payload,
												 const AddressPtr &fromAddress,
												 const OutputArray &outputs,
												 const std::string &memo,
												 bool max) {
			for (const OutputPtr &output : outputs) {
				ErrorChecker::CheckParam(!output->Addr()->Valid(), Error::CreateTransaction,
										 "invalid receiver address");

				ErrorChecker::CheckParam(output->Amount() < 0, Error::CreateTransaction,
										 "output amount should big than zero");
			}

			std::string memoFixed;

			if (!memo.empty())
				memoFixed = "type:text,msg:" + memo;

			ErrorChecker::CheckParam(!IsAssetUnique(outputs), Error::InvalidAsset, "asset is not unique in outputs");

			uint256 assetID = outputs.front()->AssetID();

			Lock();
			bool containAsset = ContainsAsset(assetID);
			Unlock();

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->CreateTxForOutputs(type, payload, outputs, fromAddress, memoFixed, max);

			if (assetID != Asset::GetELAAssetID())
				_groupedAssets[Asset::GetELAAssetID()]->AddFeeForTx(tx);

			if (_chainID == CHAINID_MAINCHAIN) {
				tx->SetVersion(Transaction::TxVersion::V09);
			}

			tx->FixIndex();

			return tx;
		}

		bool Wallet::ContainsTransaction(const TransactionPtr &tx) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return ContainsTx(tx);
		}

		bool Wallet::RegisterTransaction(const TransactionPtr &tx) {
			bool r = true, wasAdded = false;
			std::map<uint256, BigInt> changedBalance;

			bool IsReceiveTx = IsReceiveTransaction(tx);
			if (tx != nullptr && (IsReceiveTx || ((tx->IsSigned())))) {
				Lock();
				if (ContainsTx(tx) && _allTx.Insert(tx)) {
					// TODO: verify signatures when possible
					// TODO: handle tx replacement with input sequence numbers
					//       (for now, replacements appear invalid until confirmation)
					InsertTx(tx);
					if (tx->GetBlockHeight() != TX_UNCONFIRMED)
						changedBalance = BalanceAfterUpdatedTx(tx);
					else if (!tx->IsCoinBase())
						AddSpendingUTXO(tx->GetInputs());
					wasAdded = true;
				} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
					// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
					// if (tx->GetBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(tx);
					r = false;
					// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
				}
				Unlock();
			} else {
				r = false;
			}

			if (wasAdded) {
				if (tx->IsCoinBase()) {
					coinbaseTxAdded(tx);
				} else {
					UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
					UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
					txAdded(tx);
				}
			}

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);

			return r;
		}

		void Wallet::RemoveTransaction(const uint256 &txHash) {
			bool notifyUser = false, recommendRescan = false;
			std::vector<uint256> hashes;

			assert(txHash != 0);

			Lock();
			const TransactionPtr tx = _allTx.Get(txHash);

			if (tx) {
				if (!tx->IsCoinBase()) {
					for (size_t i = _transactions.size(); i > 0; i--) { // find depedent _transactions
						const TransactionPtr &t = _transactions[i - 1];
						if (t->GetBlockHeight() < tx->GetBlockHeight()) break;
						if (tx->IsEqual(*t)) continue;

						for (size_t j = 0; j < t->GetInputs().size(); j++) {
							if (t->GetInputs()[j]->TxHash() != txHash) continue;
							hashes.push_back(t->GetHash());
							break;
						}
					}
				}

				if (!hashes.empty()) {
					Unlock();
					for (size_t i = hashes.size(); i > 0; i--) {
						RemoveTransaction(hashes[i - 1]);
					}

					RemoveTransaction(txHash);
				} else {
					if (!tx->IsCoinBase()) {
						for (size_t i = _transactions.size(); i > 0; --i) {
							if (_transactions[i - 1]->IsEqual(*tx)) {
								_transactions.erase(_transactions.begin() + i - 1);
								_allTx.Remove(tx);
								break;
							}
						}
					} else {
						for (size_t i = _coinbaseTransactions.size(); i > 0; --i) {
							if (_coinbaseTransactions[i - 1]->IsEqual(*tx)) {
								_coinbaseTransactions.erase(_coinbaseTransactions.begin() + i - 1);
								_allTx.Remove(tx);
								break;
							}
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

					if (!tx->IsCoinBase()) {
						txDeleted(tx->GetHash(), notifyUser, recommendRescan);
					} else {
						coinbaseTxDeleted(tx->GetHash(), notifyUser, recommendRescan);
					}
				}
			} else {
				Unlock();
			}
		}

		void Wallet::UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			std::vector<uint256> hashes, cbHashes;
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
					bool needUpdate = false;
					if (tx->GetBlockHeight() == blockHeight && tx->GetTimestamp() == timestamp)
						continue;

					if (tx->GetBlockHeight() == TX_UNCONFIRMED && blockHeight != TX_UNCONFIRMED) {
						needUpdate = true;
						if (tx->GetTransactionType() == Transaction::registerAsset) {
							RegisterAsset *p = dynamic_cast<RegisterAsset *>(tx->GetPayload());
							if (p) payloads.push_back(p);
						}
					}

					tx->SetTimestamp(timestamp);
					tx->SetBlockHeight(blockHeight);

					if (ContainsTx(tx)) {
						if (RemoveTx(tx)) {
							if (tx->IsCoinBase()) {
								cbHashes.push_back(txHashes[i]);
							} else {
								hashes.push_back(txHashes[i]);
							}
						}
						InsertTx(tx);
						if (needUpdate)
							changedBalance = BalanceAfterUpdatedTx(tx);
					} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
						Log::warn("{} remove non-wallet tx: {}", _walletID, tx->GetHash().GetHex());
						_allTx.Remove(tx);
					}
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
				coinbaseTxUpdated(cbHashes, blockHeight, timestamp);

			if (!payloads.empty()) {
				for (i = 0; i < payloads.size(); ++i)
					assetRegistered(payloads[i]->GetAsset(), payloads[i]->GetAmount(), payloads[i]->GetController());
			}

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
		}

		TransactionPtr Wallet::TransactionForHash(const uint256 &txHash) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _allTx.Get(txHash);
		}

		size_t Wallet::GetAllTransactionCount() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _transactions.size();
		}

		bool Wallet::TransactionIsValid(const TransactionPtr &tx) {
			bool r = true;
			if (tx == nullptr || !tx->IsSigned())
				return false;

			// TODO: XXX attempted double spends should cause conflicted tx to remain unverified until they're confirmed
			// TODO: XXX conflicted tx with the same wallet outputs should be presented as the same tx to the user

#if 0
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
#endif

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
			if (!tx)
				return amount;

			for (InputArray::iterator in = tx->GetInputs().begin(); in != tx->GetInputs().end(); ++in) {
				TransactionPtr t = _allTx.Get((*in)->TxHash());
				UTXOPtr cb = nullptr;
				if (t) {
					OutputPtr o = t->OutputOfIndex((*in)->Index());
					if (o && _subAccount->ContainsAddress(o->Addr())) {
						amount += o->Amount();
					}
				}
			}

			return amount;
		}

		bool Wallet::IsReceiveTransaction(const TransactionPtr &tx) const {
			boost::mutex::scoped_lock scopedLock(lock);
			bool status = true;
			for (InputArray::iterator in = tx->GetInputs().begin(); in != tx->GetInputs().end(); ++in) {
				if ((*in)->TxHash() != 0 && ContainsInput(*in)) {
					status = false;
					break;
				}
			}

			return status;
		}

		bool Wallet::StripTransaction(const TransactionPtr &tx) const {
			const OutputArray &outputs = tx->GetOutputs();
			if (outputs.size() > 2 && outputs.size() - 1 == outputs.back()->FixedIndex() && IsReceiveTransaction(tx)) {
				size_t sizeBeforeStrip = outputs.size();
				boost::mutex::scoped_lock scopedLock(lock);
				std::vector<OutputPtr> newOutputs;
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (_subAccount->ContainsAddress((*o)->Addr()))
						newOutputs.push_back(*o);
				}

				if (newOutputs.size() != sizeBeforeStrip) {
					tx->SetOutputs(newOutputs);
					SPVLOG_DEBUG("{} strip tx {}, h: {}", _walletID, tx->GetHash().GetHex(), tx->GetBlockHeight());
					return true;
				}
			}
			return false;
		}

		AddressPtr Wallet::GetReceiveAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(1, 0)[0];
		}

		size_t Wallet::GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllAddresses(addr, start, count, internal);
		}

		size_t Wallet::GetAllDID(AddressArray &did, uint32_t start, size_t count) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllDID(did, start, count);
		}

		size_t Wallet::GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
										bool containInternal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllPublickeys(pubkeys, start, count, containInternal);
		}

		AddressPtr Wallet::GetOwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return AddressPtr(new Address(PrefixDeposit, _subAccount->OwnerPubKey()));
		}

		AddressPtr Wallet::GetCROwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return AddressPtr(new Address(PrefixDeposit, _subAccount->DIDPubKey()));
		}

		AddressPtr Wallet::GetOwnerAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return AddressPtr(new Address(PrefixStandard, _subAccount->OwnerPubKey()));
		}

		AddressArray Wallet::GetAllSpecialAddresses() const {
			AddressArray result;
			boost::mutex::scoped_lock scopedLock(lock);
			if (_subAccount->Parent()->GetSignType() != Account::MultiSign) {
				// Owner address
				result.push_back(AddressPtr(new Address(PrefixStandard, _subAccount->OwnerPubKey())));
				// Owner deposit address
				result.push_back(AddressPtr(new Address(PrefixDeposit, _subAccount->OwnerPubKey())));
				// CR Owner deposit address
				result.push_back(AddressPtr(new Address(PrefixDeposit, _subAccount->DIDPubKey())));
			}

			return result;
		}

		bytes_t Wallet::GetOwnerPublilcKey() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->OwnerPubKey();
		}

		bool Wallet::IsDepositAddress(const AddressPtr &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);

			if (_subAccount->IsProducerDepositAddress(addr))
				return true;
			if (_subAccount->IsCRDepositAddress(addr))
				return true;

			return false;
		}

		bool Wallet::ContainsAddress(const AddressPtr &address) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _subAccount->ContainsAddress(address);
		}

		void Wallet::GenerateDID() {
			_subAccount->InitDID();
		}

		nlohmann::json Wallet::GetBasicInfo() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetBasicInfo();
		}

		const std::string &Wallet::GetWalletID() const {
			return _walletID;
		}

		void Wallet::SetBlockHeight(uint32_t height) {
			boost::mutex::scoped_lock scopedLock(lock);
			_blockHeight = height;
		}

		uint32_t Wallet::LastBlockHeight() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _blockHeight;
		}

		void Wallet::SignTransaction(const TransactionPtr &tx, const std::string &payPassword) const {
			boost::mutex::scoped_lock scopedLock(lock);
			_subAccount->SignTransaction(tx, payPassword);
		}

		std::string
		Wallet::SignWithDID(const AddressPtr &did, const std::string &msg, const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithDID(did, payPasswd);
			return key.Sign(msg).getHex();
		}

		std::string Wallet::SignDigestWithDID(const AddressPtr &did, const uint256 &digest,
											  const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithDID(did, payPasswd);
			return key.Sign(digest).getHex();
		}

		bytes_t Wallet::SignWithOwnerKey(const bytes_t &msg, const std::string &payPasswd) {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->DeriveOwnerKey(payPasswd);
			return key.Sign(msg);
		}

		std::vector<TransactionPtr> Wallet::TxUnconfirmedBefore(uint32_t blockHeight) {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> result;

			for (size_t i = _transactions.size(); i > 0; --i) {
				if (_transactions[i - 1]->GetBlockHeight() < blockHeight)
					break;

				result.insert(result.begin(), _transactions[i - 1]);
			}
			for (size_t i = _coinbaseTransactions.size(); i > 0; --i) {
				if (_coinbaseTransactions[i - 1]->GetBlockHeight() < blockHeight)
					break;

				result.insert(result.begin(), _coinbaseTransactions[i - 1]);
			}

			return result;
		}

		void Wallet::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			UTXOArray recoverSpentCoinbase;
			std::vector<uint256> hashes, cbHashes;

			Lock();
			_blockHeight = blockHeight;

			for (size_t i = _coinbaseTransactions.size(); i > 0 && _coinbaseTransactions[i - 1]->GetBlockHeight() > blockHeight; --i) {
				TransactionPtr &tx = _coinbaseTransactions[i - 1];
				if (tx->GetBlockHeight() != TX_UNCONFIRMED) {
					cbHashes.push_back(tx->GetHash());
					tx->SetBlockHeight(TX_UNCONFIRMED);

					for (const OutputPtr &o : tx->GetOutputs()) {
						if (_subAccount->ContainsAddress(o->Addr())) {
							UTXOPtr u(new UTXO(tx->GetHash(), o->FixedIndex(), tx->GetTimestamp(), tx->GetBlockHeight(), o));
							_groupedAssets[o->AssetID()]->RemoveUTXO(u);
							_groupedAssets[o->AssetID()]->AddCoinBaseUTXO(u);
						}
					}
				}
			}

			for (size_t i = _transactions.size(); i > 0 && _transactions[i - 1]->GetBlockHeight() > blockHeight; --i) {
				TransactionPtr &tx = _transactions[i - 1];
				if (tx->GetBlockHeight() != TX_UNCONFIRMED) {
					tx->SetBlockHeight(TX_UNCONFIRMED);
					hashes.push_back(tx->GetHash());
					for (const OutputPtr &o : tx->GetOutputs()) {
						if (_subAccount->ContainsAddress(o->Addr())) {
							UTXOPtr u(new UTXO(tx->GetHash(), o->FixedIndex(), tx->GetTimestamp(), tx->GetBlockHeight(), o));
							_groupedAssets[o->AssetID()]->RemoveUTXO(u);
						}
					}

					for (const InputPtr &in : tx->GetInputs()) {
						TransactionPtr txInput = _allTx.Get(in->TxHash());
						if (txInput) {
							OutputPtr o = txInput->OutputOfIndex(in->Index());
							if (o && _subAccount->ContainsAddress(o->Addr())) {
								UTXOPtr u(new UTXO(txInput->GetHash(), in->Index(), txInput->GetTimestamp(), txInput->GetBlockHeight(), o));
								_spendingOutputs.insert(u);
								_groupedAssets[o->AssetID()]->AddUTXO(u);
							}
						}
					}
				}
			}
			Unlock();

			if (!cbHashes.empty())
				coinbaseTxUpdated(cbHashes, TX_UNCONFIRMED, 0);
			if (!hashes.empty())
				txUpdated(hashes, TX_UNCONFIRMED, 0);
		}

		AddressArray Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> Wallet::GetTransactions(const bytes_t &types) const {
			std::vector<TransactionPtr> result;

			boost::mutex::scoped_lock scopedLock(lock);
			size_t maxCount = _transactions.size();
			for (size_t i = 0; i < maxCount; ++i) {
				TransactionPtr tx = _transactions[i];
				for (const unsigned char &type : types) {
					if (type == tx->GetTransactionType()) {
						result.push_back(tx);
						break;
					}
				}
			}

			return result;
		}

		std::vector<TransactionPtr> Wallet::GetAllTransactions(size_t start, size_t count) const {
			std::vector<TransactionPtr> result;

			boost::mutex::scoped_lock scopedLock(lock);
			size_t maxCount = _transactions.size();
			for (size_t i = start; i < maxCount && result.size() < count; ++i)
				result.push_back(_transactions[maxCount - i - 1]);

			return result;
		}

		size_t Wallet::GetAllCoinbaseTransactionCount() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _coinbaseTransactions.size();
		}

		std::vector<TransactionPtr> Wallet::GetAllCoinBaseTransactions(size_t start, size_t count) const {
			std::vector<TransactionPtr> result;

			boost::mutex::scoped_lock scopedLock(lock);
			size_t maxCount = _coinbaseTransactions.size();
			for (size_t i = start; i < maxCount && result.size() < count; ++i)
				result.push_back(_coinbaseTransactions[maxCount - i - 1]);

			return result;
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

			const OutputArray &outputs = tx->GetOutputs();
			for (OutputArray::const_iterator it = outputs.cbegin(); !r && it != outputs.cend(); ++it) {
				if (_subAccount->ContainsAddress((*it)->Addr()))
					r = true;
			}

			for (const InputPtr &input : tx->GetInputs()) {
				if (input->TxHash() != 0 && ContainsInput(input)) {
					r = true;
					break;
				}
			}

			return r;
		}

		bool Wallet::ContainsInput(const InputPtr &in) const {
			bool r = false;
			UTXOPtr cb = nullptr;
			OutputPtr output = nullptr;

			TransactionPtr tx = _allTx.Get(in->TxHash());
			if (tx) {
				output = tx->OutputOfIndex(in->Index());
				if (output && _subAccount->ContainsAddress(output->Addr())) {
					r = true;
				}
			}

			return r;
		}

		bool Wallet::RemoveTx(const TransactionPtr &tx) {
			bool removed = false;

			if (!tx->IsCoinBase()) {
				for (std::vector<TransactionPtr>::reverse_iterator it = _transactions.rbegin();
					 it != _transactions.rend();) {
					if ((*it)->IsEqual(*tx)) {
						it = std::vector<TransactionPtr>::reverse_iterator(
							_transactions.erase((++it).base()));
						removed = true;
						break;
					} else {
						++it;
					}
				}
			} else {
				for (std::vector<TransactionPtr>::reverse_iterator it = _coinbaseTransactions.rbegin();
					 it != _coinbaseTransactions.rend();) {
					if ((*it)->IsEqual(*tx)) {
						it = std::vector<TransactionPtr>::reverse_iterator(
							_coinbaseTransactions.erase((++it).base()));
						removed = true;
						break;
					} else {
						++it;
					}
				}
			}

			return removed;
		}

		void Wallet::InsertTx(const TransactionPtr &tx) {
			if (!tx->IsCoinBase()) {
				size_t i = _transactions.size();
				while (i > 0 && TxCompare(_transactions[i - 1], tx) > 0)
					i--;
				_transactions.insert(_transactions.begin() + i, tx);
			} else {
				size_t i = _coinbaseTransactions.size();
				while (i > 0 && TxCompare(_coinbaseTransactions[i - 1], tx) > 0)
					i--;
				_coinbaseTransactions.insert(_coinbaseTransactions.begin() + i, tx);
			}
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
			if (!tx1 || !tx2)
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

		bool Wallet::IsAssetUnique(const OutputArray &outputs) const {
			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
				if (outputs.front()->AssetID() != (*o)->AssetID())
					return false;
			}

			return true;
		}

		std::map<uint256, BigInt> Wallet::BalanceAfterUpdatedTx(const TransactionPtr &tx) {
			GroupedAssetMap::iterator it;
			std::map<uint256, BigInt> changedBalance;
			if (tx->GetBlockHeight() != TX_UNCONFIRMED) {
				if (!tx->IsCoinBase()) {
					for (it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
						if (it->second->RemoveUTXO(tx->GetInputs())) {
							changedBalance[it->first] = it->second->GetBalance();
						}
						RemoveSpendingUTXO(tx->GetInputs());
					}

					for (const OutputPtr &o : tx->GetOutputs()) {
						if (_subAccount->ContainsAddress(o->Addr())) {
							_subAccount->AddUsedAddrs(o->Addr());
							const uint256 &asset = o->AssetID();
							if (ContainsAsset(asset)) {
								uint16_t n = o->FixedIndex();
								UTXOPtr utxo(new UTXO(tx->GetHash(), n, tx->GetTimestamp(), tx->GetBlockHeight(), o));
								if (_groupedAssets[asset]->AddUTXO(utxo))
									changedBalance[asset] = _groupedAssets[asset]->GetBalance();
							}
						}
					}
				} else {
					for (const OutputPtr &o : tx->GetOutputs()) {
						if (_subAccount->ContainsAddress(o->Addr())) {
							const uint256 &asset = o->AssetID();
							if (ContainsAsset(asset)) {
								uint16_t n = o->FixedIndex();
								UTXOPtr utxo(new UTXO(tx->GetHash(), n, tx->GetTimestamp(), tx->GetBlockHeight(), o));
								_groupedAssets[o->AssetID()]->AddCoinBaseUTXO(utxo);
							}
						}
					}
				}
			}

			return changedBalance;
		}

		void Wallet::BalanceAfterRemoveTx(const TransactionPtr &tx) {
			if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
				RemoveSpendingUTXO(tx->GetInputs());
			}
		}

		void Wallet::AddSpendingUTXO(const InputArray &inputs) {
			for (InputArray::const_iterator it = inputs.cbegin(); it != inputs.cend(); ++it) {
				_spendingOutputs.insert(UTXOPtr(new UTXO(*it)));
			}
		}

		void Wallet::RemoveSpendingUTXO(const InputArray &inputs) {
			for (InputArray::const_iterator input = inputs.cbegin(); input != inputs.cend(); ++input) {
				UTXOSet::iterator it;
				if ((it = _spendingOutputs.find(UTXOPtr(new UTXO(*input)))) != _spendingOutputs.end()) {
					_spendingOutputs.erase(it);
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
			if (_spendingOutputs.find(utxo) != _spendingOutputs.end()) {
				return true;
			}

			return false;
		}

		void Wallet::balanceChanged(const uint256 &asset, const BigInt &balance) {
			if (!_listener.expired()) {
				_listener.lock()->balanceChanged(asset, balance);
			}
		}

		void Wallet::coinbaseTxAdded(const TransactionPtr &tx) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinbaseTxAdded(tx);
			}
		}

		void Wallet::coinbaseTxMove(const std::vector<TransactionPtr> &txns) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinbaseTxMove(txns);
			}
		}

		void Wallet::coinbaseTxUpdated(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinbaseTxUpdated(txHashes, blockHeight, timestamp);
			}
		}

		void Wallet::coinbaseTxDeleted(const uint256 &txHash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired()) {
				_listener.lock()->onCoinbaseTxDeleted(txHash, notifyUser, recommendRescan);
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