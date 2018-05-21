// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubWallet.h"

namespace Elastos {
	namespace SDK {

		SubWalletCallback::SubWalletCallback() {

		}

		void SubWalletCallback::OnBalanceChanged(const std::string &address, double oldAmount, double newAmount) {

		}

		void SubWalletCallback::OnTransactionStatusChanged(const std::string &txid, const std::string &status,
														   uint32_t error, const std::string &desc, uint32_t confirms) {

		}

		SubWallet::SubWallet(const MasterPubKeyPtr &masterPubKey,
							 const boost::filesystem::path &dbPath,
							 uint32_t earliestPeerTime,
							 bool singleAddress,
							 const ChainParams &chainParams) {

			_walletManager = WalletManagerPtr(new WalletManager(
					masterPubKey, dbPath, earliestPeerTime, singleAddress, chainParams));
			_walletManager->registerWalletListener(this);
		}

		SubWallet::~SubWallet() {

		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			//todo complete me
			return nlohmann::json();
		}

		double SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance() / _balanceUnit;
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
			return std::__cxx11::string();
		}

		std::string
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, const std::string &payPassword) {
			return std::__cxx11::string();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			return nlohmann::json();
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {
			return std::__cxx11::string();
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
	}
}