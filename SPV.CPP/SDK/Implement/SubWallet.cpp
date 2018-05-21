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

		SubWallet::SubWallet(const KeyPtr &key, const MasterPubKeyPtr &masterPubKey) {

			//todo create wallet manager
		}

		SubWallet::~SubWallet() {

		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			return nlohmann::json();
		}

		double SubWallet::GetBalance() {
			return 0;
		}

		std::string SubWallet::CreateAddress() {
			return std::__cxx11::string();
		}

		std::string SubWallet::GetTheLastAddress() {
			return std::__cxx11::string();
		}

		std::string SubWallet::GetAllAddress() {
			return std::__cxx11::string();
		}

		double SubWallet::GetBalanceWithAddress(const std::string &address) {
			return 0;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {

		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {

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
	}
}