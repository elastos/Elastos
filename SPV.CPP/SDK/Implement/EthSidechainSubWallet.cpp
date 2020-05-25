/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "MasterWallet.h"
#include "EthSidechainSubWallet.h"
#include <Ethereum/EthereumClient.h>

namespace Elastos {
	namespace ElaWallet {

		EthSidechainSubWallet::~EthSidechainSubWallet() {
		}

		EthSidechainSubWallet::EthSidechainSubWallet(const CoinInfoPtr &info,
													 const ChainConfigPtr &config,
													 MasterWallet *parent,
													 const std::string &netType) :
			SubWallet(netType, parent, config, info) {

			std::string mnemonic = "boring head harsh green empty clip fatal typical found crane dinner timber";
			EthereumNetworkPtr network(new EthereumNetwork(netType));
			// TODO: save public key to coininfo
			_client = ClientPtr(new EthereumClient(network, parent->GetDataPath(), mnemonic));
			EthereumWalletPtr wallet = _client->_ewm->getWallet();

			SPVLOG_DEBUG("symbol: {}", wallet->getSymbol());
			SPVLOG_DEBUG("balance: {}", wallet->getBalance());
			SPVLOG_DEBUG("balance: {}", wallet->getBalance(EthereumAmount::Unit::ETHER_ETHER));
			SPVLOG_DEBUG("gas limit: {}", wallet->getDefaultGasLimit());
			SPVLOG_DEBUG("gas price: {}", wallet->getDefaultGasPrice());
			SPVLOG_DEBUG("token: {}", wallet->getToken() == nullptr);
			SPVLOG_DEBUG("account: {}", wallet->getAccount()->getPrimaryAddress());
			SPVLOG_DEBUG("transfer: {}", wallet->getTransfers().size());
			SPVLOG_DEBUG("holds ether: {}", wallet->walletHoldsEther());
		}

		std::string EthSidechainSubWallet::GetChainID() const {
			return SubWallet::GetChainID();
		}

		nlohmann::json EthSidechainSubWallet::GetBasicInfo() const {
			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetBalanceInfo() const {
			return nlohmann::json();
		}

		std::string EthSidechainSubWallet::GetBalance() const {
			return "0";
		}

		std::string EthSidechainSubWallet::GetBalanceWithAddress(const std::string &address) const {
			return "";
		}

		std::string EthSidechainSubWallet::CreateAddress() {
			return "";
		}

		nlohmann::json EthSidechainSubWallet::GetAllAddress(uint32_t start, uint32_t count, bool internal) const {
			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAllPublicKeys(uint32_t start, uint32_t count) const {
			return nlohmann::json();
		}

		void EthSidechainSubWallet::AddCallback(ISubWalletCallback *subCallback) {

		}

		void EthSidechainSubWallet::RemoveCallback() {

		}

		nlohmann::json EthSidechainSubWallet::CreateTransaction(const std::string &fromAddress,
																const std::string &toAddress,
																const std::string &amount,
																const std::string &memo) {

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAllUTXOs(uint32_t start, uint32_t count,
														  const std::string &address) const {

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::CreateConsolidateTransaction(const std::string &memo) {
			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::SignTransaction(const nlohmann::json &createdTx,
															  const std::string &payPassword) const {

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetTransactionSignedInfo(const nlohmann::json &tx) const {

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::PublishTransaction(const nlohmann::json &signedTx) {
			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAllTransaction(uint32_t start, uint32_t count,
																const std::string &txid) const {
			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAllCoinBaseTransaction(uint32_t start, uint32_t count,
																		const std::string &txID) const {

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAssetInfo(const std::string &assetID) const {
			return nlohmann::json();
		}

		bool EthSidechainSubWallet::SetFixedPeer(const std::string &address, uint16_t port) {
			return false;
		}

		void EthSidechainSubWallet::SyncStart() {

		}

		void EthSidechainSubWallet::SyncStop() {

		}

	}
}