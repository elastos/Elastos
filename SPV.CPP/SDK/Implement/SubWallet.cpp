// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRKey.h>
#include "BRKey.h"
#include "BRArray.h"

#include "SubWallet.h"
#include "MasterWallet.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace SDK {

		SubWalletCallback::~SubWalletCallback() {

		}

		void SubWalletCallback::OnBalanceChanged(const std::string &address, double oldAmount, double newAmount) {

		}

		void SubWalletCallback::OnTransactionStatusChanged(const std::string &txid, const std::string &status,
														   uint32_t error, const std::string &desc, uint32_t confirms) {

		}

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 MasterWallet *parent) :
				_parent(parent),
				_info(info) {

			fs::path subWalletDbPath = _parent->_dbRoot;
			subWalletDbPath /= _parent->_name + info.getChainId() + DB_FILE_EXTENSION;

			BRKey key;
			UInt256 chainCode;
			deriveKeyAndChain(&key, chainCode, payPassword);
			MasterPubKeyPtr masterPubKey(new MasterPubKey(key, chainCode));

			_walletManager = WalletManagerPtr(new WalletManager(
					masterPubKey, subWalletDbPath, _info.getEarliestPeerTime(), _info.getSingleAddress(), chainParams));
			_walletManager->registerWalletListener(this);
		}

		SubWallet::~SubWallet() {

		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			//todo complete me
			return nlohmann::json();
		}

		uint64_t SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance();
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		std::string SubWallet::GetTheLastAddress() {
			//todo complete me
			return "";
		}

		nlohmann::json SubWallet::GetAllAddress() {
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			nlohmann::json j;
			j["addresses"] = addresses;
			return j;
		}

		double SubWallet::GetBalanceWithAddress(const std::string &address) {
			//todo complete me
			return 0;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback));
		}

		std::string
		SubWallet::SendTransaction(const std::string &fromAddress, const std::string &toAddress, double amount,
								   double fee, const std::string &payPassword, const std::string &memo,
								   const std::string &txid) {
			return "";
		}

		std::string
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, const std::string &payPassword) {
			return std::string();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			return nlohmann::json();
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {
			return "";
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &address, const std::string &message, const std::string &signature) {
			return nlohmann::json();
		}

		void SubWallet::balanceChanged(uint64_t balance) {
			std::for_each(_callbacks.begin(), _callbacks.end(), [balance](ISubWalletCallback *callback) {
				//todo implement event
//				callback->OnBalanceChanged();
			});
		}

		void SubWallet::onTxAdded(Transaction *transaction) {
			std::for_each(_callbacks.begin(), _callbacks.end(), [transaction](ISubWalletCallback *callback) {
				//todo implement event
//				callback->OnTransactionStatusChanged()
			});
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, blockHeight, timeStamp](ISubWalletCallback *callback) {
							  //todo implement event
//				callback->OnTransactionStatusChanged()
						  });
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, notifyUser, recommendRescan](ISubWalletCallback *callback) {
							  //todo implement event
//				callback->OnTransactionStatusChanged()
						  });
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		void SubWallet::deriveKeyAndChain(BRKey *key, UInt256 &chainCode, const std::string &payPassword) {
			UInt512 seed = _parent->deriveSeed(payPassword);
			Key::deriveKeyAndChain(key, chainCode, &seed, sizeof(seed), 3, 44, _info.getIndex(), 0);
		}

		void SubWallet::signTransaction(BRTransaction *transaction, int forkId, const std::string &payPassword) {
			BRKey masterKey;
			UInt256 chainCode;
			deriveKeyAndChain(&masterKey, chainCode, payPassword);
			BRWallet *wallet = _walletManager->getWallet()->getRaw();

			uint32_t j, internalIdx[transaction->inCount], externalIdx[transaction->inCount];
			size_t i, internalCount = 0, externalCount = 0;
			int r = 0;

			assert(wallet != NULL);
			assert(transaction != NULL);

			pthread_mutex_lock(&wallet->lock);

			for (i = 0; i < transaction->inCount; i++) {
				for (j = (uint32_t) array_count(wallet->internalChain); j > 0; j--) {
					if (BRAddressEq(transaction->inputs[i].address, &wallet->internalChain[j - 1]))
						internalIdx[internalCount++] = j - 1;
				}

				for (j = (uint32_t) array_count(wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(transaction->inputs[i].address, &wallet->externalChain[j - 1]))
						externalIdx[externalCount++] = j - 1;
				}
			}

			pthread_mutex_unlock(&wallet->lock);

			BRKey keys[internalCount + externalCount];
			Key::calculatePrivateKeyList(keys, internalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_INTERNAL_CHAIN, internalIdx);
			Key::calculatePrivateKeyList(&keys[internalCount], externalCount, &masterKey.secret, &chainCode,
										 SEQUENCE_EXTERNAL_CHAIN, externalIdx);

			BRTransactionSign(transaction, forkId, keys, internalCount + externalCount);
			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
		}

	}
}