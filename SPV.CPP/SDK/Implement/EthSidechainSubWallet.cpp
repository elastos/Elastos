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
#include <WalletCore/CoinInfo.h>

namespace Elastos {
	namespace ElaWallet {

		EthSidechainSubWallet::~EthSidechainSubWallet() {
		}

		EthSidechainSubWallet::EthSidechainSubWallet(const CoinInfoPtr &info,
													 const ChainConfigPtr &config,
													 MasterWallet *parent,
													 const std::string &netType) :
			SubWallet(netType, parent, config, info) {

			_walletID = _parent->GetID() + ":" + info->GetChainID();

			// TODO: fixme later
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
			ArgInfo("{} {}", _walletID, GetFunName());

			nlohmann::json j;
			j["Info"] = nlohmann::json();
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetBalanceInfo() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			nlohmann::json j;
			j["Info"] = "not ready";
			j["Summary"] = nlohmann::json();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string EthSidechainSubWallet::GetBalance() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			std::string balance = _client->_ewm->getWallet()->getBalance();

			ArgInfo("r => {}", balance);
			return balance;
		}

		std::string EthSidechainSubWallet::GetBalanceWithAddress(const std::string &address) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("addr: {}", address);

			std::string balance = "0";

			ArgInfo("r => {}", balance);
			return balance;
		}

		std::string EthSidechainSubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletID, GetFunName());

			std::string addr = "";

			ArgInfo("r => {}", addr);
			return addr;
		}

		nlohmann::json EthSidechainSubWallet::GetAllAddress(uint32_t start, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("internal: {}", internal);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllPublicKeys(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("s: {}", start);
			ArgInfo("c: {}", count);
			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		void EthSidechainSubWallet::AddCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("callback: *");
		}

		void EthSidechainSubWallet::RemoveCallback() {
			ArgInfo("{} {}", _walletID, GetFunName());
		}

		nlohmann::json EthSidechainSubWallet::CreateTransaction(const std::string &fromAddress,
																const std::string &toAddress,
																const std::string &amount,
																const std::string &memo) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("from: {}", fromAddress);
			ArgInfo("to: {}", toAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllUTXOs(uint32_t start, uint32_t count,
														  const std::string &address) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("cnt: {}", count);
			ArgInfo("addr: {}", address);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::CreateConsolidateTransaction(const std::string &memo) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("memo: {}", memo);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::SignTransaction(const nlohmann::json &tx,
															  const std::string &payPassword) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());
			ArgInfo("passwd: *");

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetTransactionSignedInfo(const nlohmann::json &tx) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::PublishTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			nlohmann::json j;
			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllTransaction(uint32_t start, uint32_t count,
																const std::string &txid) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}, cnt: {}, txid: {}", start, count, txid);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllCoinBaseTransaction(uint32_t start, uint32_t count,
																		const std::string &txID) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}, cnt: {}, txid: {}", start, count, txID);

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAssetInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("asset: {}", assetID);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		bool EthSidechainSubWallet::SetFixedPeer(const std::string &address, uint16_t port) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("addr: {}, port: {}", address, port);

			ArgInfo("r => false");
			return false;
		}

		void EthSidechainSubWallet::SyncStart() {
			ArgInfo("{} {}", _walletID, GetFunName());
			_client->_ewm->connect();
		}

		void EthSidechainSubWallet::SyncStop() {
			ArgInfo("{} {}", _walletID, GetFunName());
			_client->_ewm->disconnect();
		}

		void EthSidechainSubWallet::Resync() {
			ArgInfo("{} {}", _walletID, GetFunName());

		}

		void EthSidechainSubWallet::StartP2P() {
			_client->_ewm->connect();
		}

		void EthSidechainSubWallet::StopP2P() {
			_client->_ewm->disconnect();
		}

	}
}