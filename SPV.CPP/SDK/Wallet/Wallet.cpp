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
#include "Database/DatabaseManager.h"

#include <ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		Wallet::Wallet(uint32_t lastBlockHeight,
					   const std::string &walletID,
					   const std::string &chainID,
					   const SubAccountPtr &subAccount,
					   const boost::shared_ptr<Wallet::Listener> &listener,
					   const DatabaseManagerPtr &database) :
			_walletID(walletID + ":" + chainID),
			_chainID(chainID),
			_blockHeight(lastBlockHeight),
			_feePerKb(DEFAULT_FEE_PER_KB),
			_subAccount(subAccount),
			_database(database) {

			std::vector<std::string> txHashDPoS, txHashCRC, txHashProposal, txHashDID;
			_listener = boost::weak_ptr<Listener>(listener);

			std::vector<UTXOPtr> utxo = loadUTXOs();
			std::vector<AssetPtr> assetArray = loadAssets();

			if (assetArray.empty()) {
				InstallDefaultAsset();
			} else {
				InstallAssets(assetArray);
			}

			AddressSet usedAddress = LoadUsedAddress();
			_subAccount->SetUsedAddresses(usedAddress);

			_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);

			std::map<uint256, TransactionPtr> txMap;
			if (database->TxTableDataMigrateDone()) {
				txMap = loadUTXOTxn();
			} else {
				std::vector<TransactionPtr> alltxns = LoadAllOldTx();
				for (TransactionPtr &tx : alltxns) {
					txMap[tx->GetHash()] = tx;
					// calculate input again
					for (InputPtr &in : tx->GetInputs()) {
						auto it = txMap.find(in->TxHash());
						if (it != txMap.end()) {
							OutputPtr o = it->second->OutputOfIndex(in->Index());
							if (o) in->FixDetail(o->Amount(), *o->Addr());
						}
					}
				}
				SaveAllTx(alltxns);
				database->SetTxTableDataMigrateDone();
			}

			for (const UTXOPtr &u : utxo) {
				auto it = txMap.find(u->Hash());
				if (it != txMap.end()) {
					TransactionPtr tx = it->second;
					OutputPtr o = tx->OutputOfIndex(u->Index());
					GroupedAssetPtr groupedAsset = GetGroupedAsset(o->AssetID());
					if (groupedAsset) {
						u->SetOutput(o);
						u->SetBlockHeight(tx->GetBlockHeight());
						u->SetTimestamp(tx->GetTimestamp());
						if (tx->IsCoinBase()) {
							groupedAsset->AddCoinBaseUTXO(u);
						} else {
							groupedAsset->AddUTXO(u);
						}
					} else {
						Log::error("asset {} not found", o->AssetID().GetHex());
					}
				} else {
					Log::error("utxo hash {} not found", u->Hash().GetHex());
				}
			}

			SPVLOG_DEBUG("{} balance info {}", _walletID, GetBalanceInfo().dump(4));
		}

		Wallet::~Wallet() {
		}

		void Wallet::ClearData() {
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				it->second->ClearData();
			}
			_spendingOutputs.clear();
			_database.lock()->ClearData();
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
			bool containAsset;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				containAsset = ContainsAsset(assetID);
			}

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
												 bool max,
												 const BigInt &fee) {
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

			bool containAsset;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				containAsset = ContainsAsset(assetID);
			}

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->CreateTxForOutputs(type, payload, outputs, fromAddress, memoFixed, max, fee);

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

		bool Wallet::ContainsTransaction(const uint256 &txHash) const {
			return containTxn(txHash);
		}

		bool Wallet::RegisterTransaction(const TransactionPtr &tx) {
			bool r = true, wasAdded = false;
			std::map<uint256, BigInt> changedBalance;
			UTXOArray utxoDeleted, utxoAdded;

			bool IsReceiveTx = IsReceiveTransaction(tx);
			if (tx != nullptr && (IsReceiveTx || ((tx->IsSigned())))) {
				boost::mutex::scoped_lock scopedLock(lock);
				if (ContainsTx(tx) && !containTxn(tx->GetHash())) {
					// TODO: verify signatures when possible
					// TODO: handle tx replacement with input sequence numbers
					//       (for now, replacements appear invalid until confirmation)
					if (tx->GetBlockHeight() != TX_UNCONFIRMED)
						changedBalance = BalanceAfterUpdatedTx(tx, utxoDeleted, utxoAdded);
					else if (!tx->IsCoinBase())
						AddSpendingUTXO(tx->GetInputs());
					wasAdded = true;
				} else { // keep track of unconfirmed non-wallet tx for invalid tx checks and child-pays-for-parent fees
					// BUG: limit total non-wallet unconfirmed tx to avoid memory exhaustion attack
					// if (tx->GetBlockHeight() == TX_UNCONFIRMED) _allTx.Insert(tx);
					r = false;
					// BUG: XXX memory leak if tx is not added to wallet->_allTx, and we can't just free it
				}
			} else {
				r = false;
			}

			if (wasAdded) {
				if (!tx->IsCoinBase()) {
					UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
					UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
				}
				txAdded(tx);
			}

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
			if (!utxoAdded.empty() || !utxoDeleted.empty())
				UTXOUpdated(utxoAdded, utxoDeleted);

			return r;
		}

		void Wallet::RemoveTransaction(const uint256 &txHash) {
			bool notifyUser = false, recommendRescan = false;
			std::vector<uint256> hashes;
			UTXOArray utxoDeleted, utxoAdded;
			std::map<uint256, BigInt> changedBalance;

			assert(txHash != 0);
			const TransactionPtr tx = loadTxn(txHash);

			if (tx) {
				std::vector<TransactionPtr> txnsAfter = loadTxnAfter(tx->GetBlockHeight());
				if (!tx->IsCoinBase()) {
					for (const TransactionPtr &t : txnsAfter) {
						if (!t->IsEqual(*tx)) {
							for (const InputPtr &in : t->GetInputs()) {
								if (in->TxHash() == txHash) {
									hashes.push_back(t->GetHash());
									break;
								}
							}
						}
					}
				}

				if (!hashes.empty()) {
					for (size_t i = hashes.size(); i > 0; i--)
						RemoveTransaction(hashes[i - 1]);
				}

				{
					boost::mutex::scoped_lock  scopedLock(lock);
					changedBalance = BalanceAfterRemoveTx(tx, utxoDeleted, utxoAdded);
				}

				// if this is for a transaction we sent, and it wasn't already known to be invalid, notify user
				if (AmountSentByTx(tx) > 0 /*&& TransactionIsValid(tx)*/) {
					recommendRescan = notifyUser = true;

					for (const InputPtr &in : tx->GetInputs()) { // only recommend a rescan if all inputs are confirmed
						TransactionPtr t = TransactionForHash(in->TxHash());
						if (t && t->GetBlockHeight() != TX_UNCONFIRMED) continue;
						recommendRescan = false;
						break;
					}
				}

				txDeleted(txHash, notifyUser, recommendRescan);
				if (!utxoAdded.empty() || !utxoDeleted.empty())
					UTXOUpdated(utxoAdded, utxoDeleted);
				for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
					balanceChanged(it->first, it->second);
			}
		}

		void Wallet::UpdateTransactions(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp) {
			std::vector<uint256> hashes;
			std::map<uint256, BigInt> changedBalance;
			UTXOArray utxoDeleted, utxoAdded;
			std::vector<RegisterAsset *> payloads;
			UTXOPtr cb;
			size_t i;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				if (blockHeight != TX_UNCONFIRMED && blockHeight > _blockHeight)
					_blockHeight = blockHeight;

				for (i = 0; i < txHashes.size(); i++) {
					TransactionPtr tx = loadTxn(txHashes[i]);
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

						if (ContainsTx(tx)) {
							tx->SetTimestamp(timestamp);
							tx->SetBlockHeight(blockHeight);
							hashes.push_back(txHashes[i]);

							if (needUpdate)
								changedBalance = BalanceAfterUpdatedTx(tx, utxoDeleted, utxoAdded);
						} else if (blockHeight != TX_UNCONFIRMED) { // remove and free confirmed non-wallet tx
							Log::warn("{} remove non-wallet tx: {}", _walletID, tx->GetHash().GetHex());
							txDeleted(txHashes[i], false, false);
						}
					}
				}

				if (!payloads.empty()) {
					for (i = 0; i < payloads.size(); ++i)
						InstallAssets({payloads[i]->GetAsset()});
				}
			} // boost::mutex::scope_lock


			txUpdated(hashes, blockHeight, timestamp);

			if (!payloads.empty()) {
				for (i = 0; i < payloads.size(); ++i)
					assetRegistered(payloads[i]->GetAsset(), payloads[i]->GetAmount(), payloads[i]->GetController());
			}

			if (!utxoAdded.empty() || !utxoDeleted.empty())
				UTXOUpdated(utxoAdded, utxoDeleted);

			for (std::map<uint256, BigInt>::iterator it = changedBalance.begin(); it != changedBalance.end(); ++it)
				balanceChanged(it->first, it->second);
		}

		TransactionPtr Wallet::TransactionForHash(const uint256 &txHash) const {
			boost::mutex::scoped_lock  scopedLock(lock);
			return loadTxn(txHash);
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

			if (!tx)
				return amount;

			for (InputArray::iterator in = tx->GetInputs().begin(); in != tx->GetInputs().end(); ++in) {
				if (!tx->IsCoinBase()) {
					TransactionPtr t = loadTxn((*in)->TxHash());
					UTXOPtr cb = nullptr;
					if (t) {
						OutputPtr o = t->OutputOfIndex((*in)->Index());
						boost::mutex::scoped_lock scopedLock(lock);
						if (o && _subAccount->ContainsAddress(o->Addr())) {
							amount += o->Amount();
						}
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

		bool Wallet::FixTransaction(const TransactionPtr &tx) const {
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

			std::map<uint256, TransactionPtr> txmap = TransactionsForInputs(tx->GetInputs());
			for (const InputPtr &in : tx->GetInputs()) {
				auto it = txmap.find(in->TxHash());
				if (it != txmap.end()) {
					OutputPtr o = it->second->OutputOfIndex(in->Index());
					if (o)
						in->FixDetail(o->Amount(), *o->Addr());
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

		size_t Wallet::GetAllCID(AddressArray &cid, uint32_t start, size_t count) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllCID(cid, start, count);
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
			return _subAccount->IsCRDepositAddress(addr);
		}

		bool Wallet::ContainsAddress(const AddressPtr &address) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _subAccount->ContainsAddress(address);
		}

		void Wallet::GenerateCID() {
			_subAccount->InitCID();
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

		void Wallet::SetTxUnconfirmedAfter(uint32_t blockHeight) {
			UTXOArray utxoDeleted, utxoAdded;
			std::vector<uint256> hashes;

			std::vector<TransactionPtr> txns;
			txns = loadTxnAfter(blockHeight);

			{
				boost::mutex::scoped_lock scopedLock(lock);
				_blockHeight = blockHeight;
				for (size_t i = txns.size(); i > 0; --i) {
					TransactionPtr &tx = txns[i - 1];
					hashes.push_back(tx->GetHash());

					for (const OutputPtr &o : tx->GetOutputs()) {
						GroupedAssetPtr groupedAsset = GetGroupedAsset(o->AssetID());
						if (groupedAsset && _subAccount->ContainsAddress(o->Addr())) {
							UTXOPtr u(
								new UTXO(tx->GetHash(), o->FixedIndex(), tx->GetTimestamp(), tx->GetBlockHeight(), o));
							if (groupedAsset->RemoveUTXO(u))
								utxoDeleted.push_back(u);
						}
					}

					for (const InputPtr &in : tx->GetInputs()) {
						TransactionPtr txInput = loadTxn(in->TxHash());
						if (txInput) {
							OutputPtr o = txInput->OutputOfIndex(in->Index());
							if (o) {
								GroupedAssetPtr groupedAsset = GetGroupedAsset(o->AssetID());
								if (groupedAsset && _subAccount->ContainsAddress(o->Addr())) {
									UTXOPtr u(new UTXO(txInput->GetHash(), in->Index(), txInput->GetTimestamp(),
													   txInput->GetBlockHeight(), o));
									_spendingOutputs.insert(u);
									bool isAdded;
									if (txInput->IsCoinBase()) {
										isAdded = groupedAsset->AddCoinBaseUTXO(u);
									} else {
										isAdded = groupedAsset->AddUTXO(u);
									}
									if (isAdded)
										utxoAdded.push_back(u);
								}
							}
						}
					}
				}
			} // boost::mutex::scope_lock

			txUpdated(hashes, TX_UNCONFIRMED, 0);
			if (!utxoAdded.empty() || !utxoDeleted.empty())
				UTXOUpdated(utxoAdded, utxoDeleted);
		}

		AddressArray Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		std::vector<TransactionPtr> Wallet::GetDPoSTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> txns;
			if (!_database.expired()) {
				std::vector<TxEntity> entities;
				if (_chainID == CHAINID_MAINCHAIN &&
					_database.lock()->GetTx(entities, Transaction::GetDPoSTxTypes())) {
					for (TxEntity &e : entities) {
						TransactionPtr tx = TransactionPtr(new Transaction());

						if (tx->Decode(e)) {
							assert(e.GetTxHash() == tx->GetHash().GetHex());
							txns.push_back(tx);
						} else {
							Log::error("decode did tx");
							break;
						}
					}
				}
			}

			return txns;
		}

		std::vector<TransactionPtr> Wallet::GetCRCTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> txns;
			if (!_database.expired()) {
				std::vector<TxEntity> entities;
				if (_chainID == CHAINID_MAINCHAIN &&
					_database.lock()->GetTx(entities, Transaction::GetCRCTxTypes())) {
					for (TxEntity &e : entities) {
						TransactionPtr tx = TransactionPtr(new Transaction());

						if (tx->Decode(e)) {
							assert(e.GetTxHash() == tx->GetHash().GetHex());
							txns.push_back(tx);
						} else {
							Log::error("decode crc tx");
							break;
						}
					}
				}
			}

			return txns;
		}

		std::vector<TransactionPtr> Wallet::GetProposalTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> txns;
			if (!_database.expired()) {
				std::vector<TxEntity> entities;
				if (_chainID == CHAINID_MAINCHAIN &&
					_database.lock()->GetTx(entities, Transaction::GetProposalTypes())) {
					for (TxEntity &e : entities) {
						TransactionPtr tx = TransactionPtr(new Transaction());

						if (tx->Decode(e)) {
							assert(e.GetTxHash() == tx->GetHash().GetHex());
							txns.push_back(tx);
						} else {
							Log::error("decode proposal tx");
							break;
						}
					}
				}
			}

			return txns;
		}

		std::vector<TransactionPtr> Wallet::GetDIDTransactions() const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> txns;
			if (!_database.expired()) {
				std::vector<TxEntity> entities;
				if (_chainID == CHAINID_IDCHAIN &&
					_database.lock()->GetTx(entities, IDTransaction::GetIDTxTypes())) {
					for (TxEntity &e : entities) {
						TransactionPtr tx = TransactionPtr(new IDTransaction());

						if (tx->Decode(e)) {
							assert(e.GetTxHash() == tx->GetHash().GetHex());
							txns.push_back(tx);
						} else {
							Log::error("decode did tx");
							break;
						}
					}
				}
			}

			return txns;
		}

		std::vector<TransactionPtr> Wallet::GetCoinbaseTransactions(size_t offset, size_t limit, bool invertMatch) const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::vector<TransactionPtr> txns;

			if (!_database.expired()) {
				DatabaseManagerPtr db = _database.lock();
				std::vector<TxEntity> entities;

				if (db->GetTx(entities, Transaction::coinBase, invertMatch, offset, limit, true)) {
					for (TxEntity &e :entities) {
						TransactionPtr tx;
						if (_chainID == CHAINID_MAINCHAIN) {
							tx = TransactionPtr(new Transaction());
						} else if (_chainID == CHAINID_IDCHAIN || _chainID == CHAINID_TOKENCHAIN) {
							tx = TransactionPtr(new IDTransaction());
						}

						if (tx->Decode(e)) {
							assert(e.GetTxHash() == tx->GetHash().GetHex());
							txns.push_back(tx);
						}
					}
				}
			}

			return txns;
		}

		std::map<uint256, TransactionPtr> Wallet::TransactionsForInputs(const InputArray &inputs) const {
			boost::mutex::scoped_lock scopedLock(lock);
			std::map<uint256, TransactionPtr> txns;
			if (_database.expired())
				return txns;

			DatabaseManagerPtr db = _database.lock();
			std::set<std::string> hashes;
			for (const InputPtr &in : inputs)
				hashes.insert(in->TxHash().GetHex());

			std::vector<TxEntity> entities;
			if (db->GetTx(entities, hashes)) {
				for (TxEntity &e :entities) {
					TransactionPtr tx;
					if (_chainID == CHAINID_MAINCHAIN) {
						tx = TransactionPtr(new Transaction());
					} else if (_chainID == CHAINID_IDCHAIN || _chainID == CHAINID_TOKENCHAIN) {
						tx = TransactionPtr(new IDTransaction());
					}

					if (tx->Decode(e)) {
						assert(e.GetTxHash() == tx->GetHash().GetHex());
						txns[tx->GetHash()] = tx;
					}
				}
			}

			return txns;
		}

		std::vector<TransactionPtr> Wallet::TxUnconfirmedBefore(uint32_t blockHeight) {
			boost::mutex::scoped_lock scopedLock(lock);
			return loadTxnAfter(blockHeight);
		}

		size_t Wallet::GetCoinbaseTransactionCount(bool invertMatch) const {
			size_t cnt = 0;
			boost::mutex::scoped_lock scopedLock(lock);
			if (!_database.expired()) {
				cnt = _database.lock()->GetTxCnt(Transaction::coinBase, invertMatch);
			}
			return cnt;
		}

		time_t Wallet::GetEarliestTxTimestamp() const {
			boost::mutex::scoped_lock scopedLock(lock);
			if (!_database.expired())
				return _database.lock()->GetEarliestTxTimestamp();

			return 0;
		}

		void Wallet::DeleteTransaction(const uint256 &hash) {
			boost::mutex::scoped_lock scopedLock(lock);
			deleteTx(hash);
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

		GroupedAssetPtr Wallet::GetGroupedAsset(const uint256 &assetID) const {
			GroupedAssetMap::iterator it = _groupedAssets.find(assetID);
			if (it != _groupedAssets.end())
				return it->second;

			return nullptr;
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

			TransactionPtr tx = loadTxn(in->TxHash());
			if (tx) {
				output = tx->OutputOfIndex(in->Index());
				if (output && _subAccount->ContainsAddress(output->Addr())) {
					r = true;
				}
			}

			return r;
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

		std::map<uint256, BigInt> Wallet::BalanceAfterUpdatedTx(const TransactionPtr &tx, UTXOArray &deleted, UTXOArray &added) {
			GroupedAssetMap::iterator it;
			std::map<uint256, BigInt> changedBalance;
			UTXOArray assetUTXODeleted, assetUTXOAdded;
			AddressSet usedAddresses;
			if (tx->GetBlockHeight() != TX_UNCONFIRMED) {
				for (it = _groupedAssets.begin(); it != _groupedAssets.end() && !tx->IsCoinBase(); ++it) {
					assetUTXODeleted = it->second->RemoveUTXO(tx->GetInputs());
					if (!assetUTXODeleted.empty()) {
						deleted.insert(deleted.end(), assetUTXODeleted.begin(), assetUTXODeleted.end());
						changedBalance[it->first] = it->second->GetBalance();
					}
					RemoveSpendingUTXO(tx->GetInputs());
				}

				for (const OutputPtr &o : tx->GetOutputs()) {
					if (_subAccount->ContainsAddress(o->Addr())) {
						if (_subAccount->AddUsedAddress(o->Addr()))
							usedAddresses.insert(o->Addr());
						GroupedAssetPtr groupedAsset = GetGroupedAsset(o->AssetID());
						if (groupedAsset) {
							uint16_t n = o->FixedIndex();
							UTXOPtr utxo(new UTXO(tx->GetHash(), n, tx->GetTimestamp(), tx->GetBlockHeight(), o));
							bool isAdded;
							if (tx->IsCoinBase()) {
								isAdded = groupedAsset->AddCoinBaseUTXO(utxo);
							} else {
								isAdded = groupedAsset->AddUTXO(utxo);
							}
							if (isAdded) {
								added.push_back(utxo);
								changedBalance[o->AssetID()] = groupedAsset->GetBalance();
							}
						}
					}
				}
			}

			if (!usedAddresses.empty())
				usedAddressSaved(usedAddresses);

			return changedBalance;
		}

		std::map<uint256, BigInt> Wallet::BalanceAfterRemoveTx(const TransactionPtr &tx, UTXOArray &deleted, UTXOArray &added) {
			std::map<uint256, BigInt> changedBalance;

			if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
				RemoveSpendingUTXO(tx->GetInputs());
			} else {
				for (const OutputPtr o : tx->GetOutputs()) {
					GroupedAssetPtr groupedAsset = GetGroupedAsset(o->AssetID());
					if (groupedAsset && _subAccount->ContainsAddress(o->Addr())) {
						UTXOPtr u(new UTXO(tx->GetHash(), o->FixedIndex(), tx->GetTimestamp(), tx->GetBlockHeight(), o));
						if (groupedAsset->RemoveUTXO(u)) {
							deleted.push_back(u);
							changedBalance[o->AssetID()] = groupedAsset->GetBalance();
						}
					}
				}

				for (const InputPtr &in : tx->GetInputs()) {
					TransactionPtr txInput = loadTxn(in->TxHash());
					if (txInput && txInput->GetBlockHeight() != TX_UNCONFIRMED) {
						OutputPtr o = txInput->OutputOfIndex(in->Index());
						if (o) {
							GroupedAssetPtr groupedAsset = GetGroupedAsset(o->AssetID());
							if (groupedAsset && _subAccount->ContainsAddress(o->Addr())) {
								UTXOPtr u(new UTXO(txInput->GetHash(), in->Index(), txInput->GetTimestamp(),
												   txInput->GetBlockHeight(), o));
								bool isAdded;
								if (txInput->IsCoinBase()) {
									isAdded = groupedAsset->AddCoinBaseUTXO(u);
								} else {
									isAdded = groupedAsset->AddUTXO(u);
								}
								if (isAdded) {
									added.push_back(u);
									changedBalance[o->AssetID()] = groupedAsset->GetBalance();
								}
							}
						}
					}
				}
			}

			return changedBalance;
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

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
					if (it->second->UpdateLockedBalance()) {
						changedBalance[it->first] = it->second->GetBalance();
					}
				}
			} // boost::mutex::scope_lock

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
			return _spendingOutputs.find(utxo) != _spendingOutputs.end();
		}

		void Wallet::UTXOUpdated(const UTXOArray &utxoAdded, const UTXOArray &utxoDeleted, bool replace) {
			if (!_database.expired()) {
				std::vector<UTXOEntity> added, deleted;

				added.reserve(utxoAdded.size());
				for (const UTXOPtr &u : utxoAdded)
					added.emplace_back(u->Hash().GetHex(), u->Index());

				for (const UTXOPtr &u : utxoDeleted)
					deleted.emplace_back(u->Hash().GetHex(), u->Index());

				_database.lock()->UTXOUpdate(added, deleted, replace);
			}
		}

		void Wallet::balanceChanged(const uint256 &asset, const BigInt &balance) {
			if (!_listener.expired())
				_listener.lock()->onBalanceChanged(asset, balance);
		}

		void Wallet::txAdded(const TransactionPtr &tx) {
			if (!_listener.expired())
				_listener.lock()->onTxAdded(tx);
		}

		void Wallet::txUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			if (!_listener.expired())
				_listener.lock()->onTxUpdated(hashes, blockHeight, timestamp);
		}

		void Wallet::txDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			if (!_listener.expired())
				_listener.lock()->onTxDeleted(hash, notifyUser, recommendRescan);
		}

		void Wallet::assetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			if (!_listener.expired())
				_listener.lock()->onAssetRegistered(asset, amount, controller);
		}

		std::vector<TransactionPtr> Wallet::LoadAllOldTx() const {
			std::vector<TransactionPtr> txns;
			if (!_database.expired()) {
				std::vector<TxOldEntity> entities;
				DatabaseManagerPtr db = _database.lock();

				db->GetAllOldTx(entities);

				for (TxOldEntity &e : entities) {
					TransactionPtr tx;
					if (_chainID == CHAINID_MAINCHAIN) {
						tx = TransactionPtr(new Transaction());
					} else if (_chainID == CHAINID_IDCHAIN || _chainID == CHAINID_TOKENCHAIN) {
						tx = TransactionPtr(new IDTransaction());
					}
					ByteStream stream(e.GetBuf());
					if (e.GetISO() == ISO_OLD) {
						tx->Deserialize(stream);
					} else if (e.GetISO() == ISO) {
						tx->DeserializeOld(stream, true);
					}
					tx->SetBlockHeight(e.GetBlockHeight());
					tx->SetTimestamp(e.GetTimestamp());
					tx->SetHash(uint256(e.GetTxHash()));
					txns.push_back(tx);
				}

				std::sort(txns.begin(), txns.end(), [](const TransactionPtr &x, const TransactionPtr &y) {
					return x->GetBlockHeight() < y->GetBlockHeight();
				});
			}

			return txns;
		}

		bool Wallet::SaveAllTx(const std::vector<TransactionPtr> &txns) {
			if (!_database.expired()) {
				std::vector<TxEntity> entities;
				entities.reserve(txns.size());

				for (const TransactionPtr &tx: txns) {
					TxEntity e;
					tx->Encode(e);
					entities.push_back(e);
				}

				if (!_database.lock()->PutTx(entities)) {
					Log::error("save all tx fail");
					return false;
				}
				return true;
			}

			return false;
		}

		std::vector<TransactionPtr> Wallet::loadTxnAfter(uint32_t height) const {
			std::vector<TransactionPtr> txns;

			if (!_database.expired()) {
				DatabaseManagerPtr db = _database.lock();

				std::vector<TxEntity> entities;
				if (db->GetTx(entities, height)) {
					for (TxEntity &e : entities) {
						TransactionPtr tx = nullptr;
						if (_chainID == CHAINID_MAINCHAIN) {
							tx = TransactionPtr(new Transaction());
						} else if (_chainID == CHAINID_IDCHAIN || _chainID == CHAINID_TOKENCHAIN) {
							tx = TransactionPtr(new IDTransaction());
						}

						if (tx->Decode(e)) {
							assert(e.GetTxHash() == tx->GetHash().GetHex());
							txns.push_back(tx);
						} else {
							Log::error("load tx after height {} fail", height);
							break;
						}
					}
				}
			}

			return txns;
		}

		TransactionPtr Wallet::loadTxn(const uint256 &hash) const {
			TransactionPtr tx = nullptr;
			if (!_database.expired()) {
				DatabaseManagerPtr db = _database.lock();
				std::vector<TxEntity> entities;
				std::string h = hash.GetHex();
				if (db->GetTx(entities, {h}) && !entities.empty()) {
					if (_chainID == CHAINID_MAINCHAIN) {
						tx = TransactionPtr(new Transaction());
					} else if (_chainID == CHAINID_IDCHAIN || _chainID == CHAINID_TOKENCHAIN) {
						tx = TransactionPtr(new IDTransaction());
					}

					if (tx->Decode(entities[0])) {
						assert(entities[0].GetTxHash() == tx->GetHash().GetHex());
						return tx;
					} else {
						tx = nullptr;
						Log::error("load tx {} fail", h);
					}
				}
			}

			return tx;
		}

		bool Wallet::containTxn(const uint256 &hash) const {
			if (!_database.expired())
				return _database.lock()->ContainTx(hash.GetHex());

			return false;
		}

		void Wallet::usedAddressSaved(const AddressSet &usedAddress, bool replace) {
			if (!_database.expired()) {
				std::vector<std::string> addresses;
				for (const AddressPtr &a : usedAddress)
					addresses.push_back(a->String());
				_database.lock()->PutUsedAddresses(addresses, replace);
			}
		}

		std::map<uint256, TransactionPtr> Wallet::loadUTXOTxn() const {
			std::map<uint256, TransactionPtr> txns;
			if (!_database.expired()) {
				DatabaseManagerPtr db = _database.lock();
				std::vector<TxEntity> entities;
				if (db->GetUTXOTx(entities)) {
					for (TxEntity &entity : entities) {
						TransactionPtr tx;
						if (_chainID == CHAINID_MAINCHAIN) {
							tx = TransactionPtr(new Transaction());
						} else if (_chainID == CHAINID_IDCHAIN || _chainID == CHAINID_TOKENCHAIN) {
							tx = TransactionPtr(new IDTransaction());
						}

						if (tx->Decode(entity)) {
							assert(entity.GetTxHash() == tx->GetHash().GetHex());
							txns[tx->GetHash()] = tx;
						}
					}
				}

				return txns;
			}

			return txns;
		}

		bool Wallet::deleteTx(const uint256 &hash) {
			if (!_database.expired()) {
				return _database.lock()->DeleteTx(hash.GetHex());
			}
			return false;
		}

		std::vector<UTXOPtr> Wallet::loadUTXOs() const {
			if (!_database.expired()) {
				std::vector<UTXOEntity> entities = _database.lock()->GetUTXOs();
				std::vector<UTXOPtr> allUTXOs;

				for (UTXOEntity &entity : entities) {
					UTXOPtr u(new UTXO(uint256(entity.Hash()), entity.Index()));
					allUTXOs.push_back(u);
				}

				return allUTXOs;
			}

			return {};
		}

		std::vector<AssetPtr> Wallet::loadAssets() {
			if (!_database.expired()) {
				std::vector<AssetPtr> assets;
				std::vector<AssetEntity> assetsEntity = _database.lock()->GetAllAssets();

				for (size_t i = 0; i < assetsEntity.size(); ++i) {
					ByteStream stream(assetsEntity[i].Asset);
					AssetPtr asset(new Asset());
					if (asset->Deserialize(stream)) {
						asset->SetHash(uint256(assetsEntity[i].AssetID));
						assets.push_back(asset);
					}
				}

				return assets;
			}

			return {};
		}

		AddressSet Wallet::LoadUsedAddress() const {
			if (!_database.expired()) {

				AddressSet usedAddress;
				std::vector<std::string> usedAddr = _database.lock()->GetUsedAddresses();

				for (const std::string &addr : usedAddr)
					usedAddress.insert(AddressPtr(new Address(addr)));

				return usedAddress;
			}

			return {};
		}

	}
}