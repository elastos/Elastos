// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <SDK/ELACoreExt/ELATxOutput.h>
#include <Core/BRTransaction.h>

#include "BRTransaction.h"
#include "BRWallet.h"
#include "BRKey.h"
#include "BRArray.h"

#include "Enviroment.h"
#include "SubWallet.h"
#include "MasterWallet.h"
#include "SubWalletCallback.h"
#include "Utils.h"
#include "Log.h"
#include "TransactionOutput.h"
#include "ElaPeerConfig.h"
#include "ParamChecker.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace SDK {

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 MasterWallet *parent) :
				_parent(parent),
				_info(info) {

			fs::path subWalletDbPath = Enviroment::GetRootPath();
			subWalletDbPath /= parent->GetId();
			subWalletDbPath /= info.getChainId() + DB_FILE_EXTENSION;

			nlohmann::json peerConfig =
					info.getWalletType() == Mainchain || info.getWalletType() == Normal ? ElaPeerConfig
																						: readPeerConfig();

			MasterPubKeyPtr masterPubKey = nullptr;
			if (!payPassword.empty()) {
				BRKey key;
				UInt256 chainCode;
				deriveKeyAndChain(&key, chainCode, payPassword);

				char rawKey[BRKeyPrivKey(&key, nullptr, 0)];
				BRKeyPrivKey(&key, rawKey, sizeof(rawKey));

				CMBlock ret(sizeof(rawKey));
				memcpy(ret, &rawKey, sizeof(rawKey));
				CMBlock encryptedData = Utils::encrypt(ret, payPassword);

				_info.setEncryptedKey(Utils::encodeHex(encryptedData));
				_info.setChainCode(Utils::UInt256ToString(chainCode));

				masterPubKey.reset(new MasterPubKey(key, chainCode));

			} else {
				ParamChecker::checkNotEmpty(_info.getEncryptedKey(), false);
				ParamChecker::checkNotEmpty(_info.getChainCode(), false);

				CMBlock keyRaw = Utils::decodeHex(_info.getEncryptedKey());
				BRKey key;
				BRKeySetPrivKey(&key, (char *) (void *) keyRaw);

				masterPubKey.reset(new MasterPubKey(key, Utils::UInt256FromString(_info.getChainCode())));
			}

			_walletManager = WalletManagerPtr(
					new WalletManager(masterPubKey, subWalletDbPath, peerConfig,
									  _info.getEarliestPeerTime(),
									  _info.getSingleAddress(), _info.getForkId(), chainParams));
			_walletManager->registerWalletListener(this);

			if (info.getFeePerKb() > 0) {
				_walletManager->getWallet()->setFeePerKb(info.getFeePerKb());
			}
		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainId() const {
			return _info.getChainId();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			return _walletManager->getWallet()->GetBalanceInfo();
		}

		uint64_t SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance();
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) {
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			uint32_t end = std::min(start + count, (uint32_t) addresses.size());
			std::vector<std::string> results(addresses.begin() + start, addresses.begin() + end);
			nlohmann::json j;
			j["Addresses"] = addresses;
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address) {
			return _walletManager->getWallet()->GetBalanceWithAddress(address);
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		nlohmann::json
		SubWallet::SendTransaction(const std::string &fromAddress, const std::string &toAddress, uint64_t amount,
								   uint64_t fee, const std::string &payPassword, const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Normal, fromAddress, toAddress, amount, fee, memo));
			TransactionPtr transaction = createTransaction(txParam.get());
			if (!transaction)
				throw std::logic_error("create transaction error.");

			return sendTransactionInternal(transaction, payPassword);
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);

			size_t maxCount = count;
			pthread_mutex_lock(&wallet->lock);
			if (array_count(wallet->transactions) < start + count)
				maxCount = array_count(wallet->transactions) - start;

			BRTransaction *transactions[maxCount];
			uint32_t realCount = 0;
			for (size_t i = 0; i < maxCount; i++) {
				if (!filterByAddressOrTxId(wallet->transactions[i + start], addressOrTxid)) continue;
				transactions[realCount] = wallet->transactions[i + start];
				realCount++;
			}
			pthread_mutex_unlock(&wallet->lock);

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				TransactionPtr transactionPtr(new Transaction((ELATransaction *)transactions[i], false));
				jsonList[i] = transactionPtr->toJson();
			}
			nlohmann::json j;
			j["Transactions"] = jsonList;
			return j;
		}

		boost::shared_ptr<Transaction> SubWallet::createTransaction(TxParam *param) const {
			//todo consider the situation of from address and fee not null
			//todo initialize asset id if null
			TransactionPtr ptr = nullptr;
			if (param->getFee() > 0 || param->getFromAddress().empty() == true) {
				ptr = _walletManager->getWallet()->createTransaction(param->getFromAddress(), param->getFee(),
																	 param->getAmount(), param->getToAddress());
			} else {
				Address address(param->getToAddress());
				ptr = _walletManager->getWallet()->createTransaction(param->getAmount(), address);
			}
			if (!ptr) return nullptr;

			ptr->setTransactionType(ELATransaction::TransferAsset);
			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			return ptr;
		}

		nlohmann::json SubWallet::sendTransactionInternal(const boost::shared_ptr<Transaction> &transaction,
													   const std::string &payPassword) {
			signTransaction(transaction, _info.getForkId(), payPassword);
			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = Utils::UInt256ToString(transaction->getHash());
			j["Fee"] = transaction->getStandardFee();
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &transaction) {
			_walletManager->getPeerManager()->publishTransaction(transaction);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {

			Key key = deriveKey(payPassword);
			UInt256 md;
			BRSHA256(&md, message.c_str(), message.size());
			CMBlock signedData = key.sign(md);

			char *data = new char[signedData.GetSize()];
			memcpy(data, signedData, signedData.GetSize());
			std::string singedMsg(data, signedData.GetSize());
			return singedMsg;
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			return _parent->CheckSign(publicKey, message, signature);
		}

		void SubWallet::balanceChanged(uint64_t balance) {

		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;

			_confirmingTxs[Utils::UInt256ToString(transaction->getHash())] = transaction;

			fireTransactionStatusChanged(Utils::UInt256ToString(transaction->getHash()),
										 SubWalletCallback::convertToString(SubWalletCallback::Added),
										 transaction->toJson(), 0);
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			if (_confirmingTxs.find(hash) == _confirmingTxs.end()) {
				_confirmingTxs[hash] = _walletManager->getWallet()->transactionForHash(Utils::UInt256FromString(hash));
			}

			uint32_t confirm = blockHeight - _confirmingTxs[hash]->getBlockHeight();
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Updated),
										 _confirmingTxs[hash]->toJson(), confirm);
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Deleted),
										 nlohmann::json(), 0);
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		Key SubWallet::deriveKey(const std::string &payPassword) {
			CMBlock raw = Utils::decodeHex(_info.getEncryptedKey());
			CMBlock keyData = Utils::decrypt(raw, payPassword);
			ParamChecker::checkDataNotEmpty(keyData);

			Key key;
			char stmp[keyData.GetSize()];
			memcpy(stmp, keyData, keyData.GetSize());
			std::string secret(stmp, keyData.GetSize());
			key.setPrivKey(secret);
			return key;
		}

		void SubWallet::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword) {
			UInt512 seed = _parent->deriveSeed(payPassword);
			Key::deriveKeyAndChain(key, chainCode, &seed, sizeof(seed), 3, 44, _info.getIndex(), 0);
		}

		void SubWallet::signTransaction(const boost::shared_ptr<Transaction> &transaction, int forkId,
										const std::string &payPassword) {
			assert(transaction != nullptr);
			BRKey masterKey;
			UInt256 chainCode;
			deriveKeyAndChain(&masterKey, chainCode, payPassword);
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);

			BRTransaction *tx = transaction->getRaw();
			uint32_t j, internalIdx[tx->inCount], externalIdx[tx->inCount];
			size_t i, internalCount = 0, externalCount = 0;
			int r = 0;


			pthread_mutex_lock(&wallet->lock);

			for (i = 0; i < tx->inCount; i++) {
				for (j = (uint32_t) array_count(wallet->internalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &wallet->internalChain[j - 1]))
						internalIdx[internalCount++] = j - 1;
				}

				for (j = (uint32_t) array_count(wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &wallet->externalChain[j - 1]))
						externalIdx[externalCount++] = j - 1;
				}
			}

			pthread_mutex_unlock(&wallet->lock);

			BRKey keys[internalCount + externalCount];
			Key::calculatePrivateKeyList(keys, internalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_INTERNAL_CHAIN, internalIdx);
			Key::calculatePrivateKeyList(&keys[internalCount], externalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_EXTERNAL_CHAIN, externalIdx);

			if (tx) {
				WrapperList<Key, BRKey> keyList;
				for (i = 0; i < internalCount + externalCount; ++i) {
					Key key(new BRKey);
					memcpy(key.getRaw(), &keys[i], sizeof(BRKey));
					keyList.push_back(key);
				}
				r = transaction->sign(keyList, forkId);
			}

			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
		}

		std::string SubWallet::CreateMultiSignAddress(const nlohmann::json &multiPublicKeyJson, uint32_t totalSignNum,
													  uint32_t requiredSignNum) {
			//todo complete me
			return "";
		}

		nlohmann::json
		SubWallet::GenerateMultiSignTransaction(const std::string &fromAddress, const std::string &toAddress,
												uint64_t amount, uint64_t fee, const std::string &payPassword,
												const std::string &memo) {
			//todo complete me
			return nlohmann::json();
		}

		nlohmann::json
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, const std::string &payPassword) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(transactionJson);

			verifyRawTransaction(transaction);
			completeTransaction(transaction);

			return sendTransactionInternal(transaction, payPassword);
		}

		void SubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (!checkTransactionOutput(transaction)) {
				throw std::logic_error("checkTransactionOutput error.");
			}

			if (!checkTransactionAttribute(transaction)) {
				throw std::logic_error("checkTransactionAttribute error.");
			}

			if (!checkTransactionProgram(transaction)) {
				throw std::logic_error("checkTransactionProgram error.");
			}

			if (!checkTransactionPayload(transaction)) {
				throw std::logic_error("checkTransactionPayload error.");
			}
		}

		void SubWallet::completeTransaction(const TransactionPtr &transaction) {
			if (transaction->getRaw()->inCount <= 0) {
				completedTransactionInputs(transaction);
			}

			uint64_t inputFee = _walletManager->getWallet()->getInputsFee(transaction);
//			assert(inputFee < UINT64_MAX);
			uint64_t outputFee = _walletManager->getWallet()->getOutputFee(transaction);

			if (inputFee - outputFee > 0) {
				completedTransactionOutputs(transaction, inputFee - outputFee);
			}

			completedTransactionAssetID(transaction);

			completedTransactionPayload(transaction);
		}

		bool SubWallet::filterByAddressOrTxId(BRTransaction *transaction, const std::string &addressOrTxid) {
			//todo complete me
			return true;
		}

		bool SubWallet::checkTransactionOutput(const TransactionPtr &transaction) {

			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				return false;
			}
			for (size_t i = 0; i < size; ++i) {
				TransactionOutputPtr output = outputs[i];
				if (!Address::UInt168IsValid(output->getProgramHash())) {
					return false;
				}
			}
			return true;

		}

		bool SubWallet::checkTransactionAttribute(const TransactionPtr &transaction) {
			const std::vector<AttributePtr> attributes = transaction->getAttributes();
			size_t size = attributes.size();
			for (size_t i = 0; i < size; ++i) {
				AttributePtr attr = attributes[i];
				if (!attr->isValid()) {
					return false;
				}
			}
			return true;
		}

		bool SubWallet::checkTransactionProgram(const TransactionPtr &transaction) {
			const std::vector<ProgramPtr> programs = transaction->getPrograms();
			size_t size = programs.size();
			for (size_t i = 0; i < size; ++i) {
				if (!programs[i]->isValid()) {
					return false;
				}
			}

			return true;
		}

		bool SubWallet::checkTransactionPayload(const TransactionPtr &transaction) {
			const PayloadPtr payloadPtr = transaction->getPayload();
			return payloadPtr->isValid();
		}

		void SubWallet::completedTransactionInputs(const TransactionPtr &transaction) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			BRUTXO *o = nullptr;
			ELATransaction *tx = nullptr;
			for (size_t i = 0; i < array_count(wallet->utxos); i++) {
				o = &wallet->utxos[i];
				tx = (ELATransaction *)BRSetGet(wallet->allTx, o);
				if (! tx || o->n >= tx->raw.outCount) continue;

				CMBlock script = tx->outputs[o->n]->getScript();
				transaction->addInput(tx->raw.txHash, o->n, tx->outputs[o->n]->getAmount(), script, CMBlock(), TXIN_SEQUENCE);
			}
		}

		void SubWallet::completedTransactionOutputs(const TransactionPtr &transaction, uint64_t amount) {
			ELATxOutput *o = new ELATxOutput();
			uint64_t maxAmount = _walletManager->getWallet()->getMaxOutputAmount();
			o->raw.amount = (amount < maxAmount) ? amount : maxAmount;
			std::string addr = _walletManager->getWallet()->getReceiveAddress();
			memcpy(o->raw.address, addr.data(), addr.size());

			BRTxOutputSetAddress((BRTxOutput *)o, addr.c_str());

			TransactionOutput *output = new TransactionOutput(o);
			transaction->addOutput(output);
		}

		void SubWallet::completedTransactionAssetID(const TransactionPtr &transaction) {
			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				throw std::logic_error("completedTransactionAssetID transaction has't outputs");
			}

			UInt256 zero = UINT256_ZERO;
			UInt256 assetID = Key::getSystemAssetId();

			for (size_t i = 0; i < size; ++i) {
				TransactionOutputPtr output = outputs[i];
				if (UInt256Eq(&output->getAssetId(), &zero) == 1) {
					output->setAssetId(assetID);
				}
			}
		}

		void SubWallet::completedTransactionPayload(const TransactionPtr &transaction) {

		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {
				fireTransactionStatusChanged(it->first, SubWalletCallback::convertToString(SubWalletCallback::Updated),
											 it->second->toJson(), blockHeight - it->second->getBlockHeight());
			}

			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end();) {
				if (blockHeight - it->second->getBlockHeight() >= 6)
					_confirmingTxs.erase(it++);
				else
					++it;
			}
		}

		void SubWallet::fireTransactionStatusChanged(const std::string &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&txid, &status, &desc, confirms](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(txid, status, desc, confirms);
						  });
		}

		nlohmann::json SubWallet::readPeerConfig() {
			//fixme read from main chain(ela)
			nlohmann::json idChainPeerConfig =
					R"(
						  {
							"MagicNumber": 7630401,
							"KnowingPeers":
							[
								{
									"Address": "127.0.0.1",
									"Port": 20866,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								}
							]
						}
					)"_json;
			return idChainPeerConfig;
		}

	}
}