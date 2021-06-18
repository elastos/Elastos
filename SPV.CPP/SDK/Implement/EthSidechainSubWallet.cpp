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
#include <Common/Log.h>
#include <Ethereum/EthereumClient.h>
#include <ethereum/ewm/BREthereumAccount.h>
#include <WalletCore/CoinInfo.h>
#include <Common/ErrorChecker.h>
#include <Common/hash.h>

namespace Elastos {
	namespace ElaWallet {

		EthSidechainSubWallet::~EthSidechainSubWallet() {
		}

		nlohmann::json EthSidechainSubWallet::CreateTransfer(const std::string &targetAddress,
															 const std::string &amount,
															 EthereumAmountUnit amountUnit,
															 uint64_t nonce) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("target: {}", targetAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("amountUnit: {}", amountUnit);
			ArgInfo("nonce: {}", nonce);

			if (amountUnit != TOKEN_DECIMAL &&
				amountUnit != TOKEN_INTEGER &&
				amountUnit != ETHER_WEI &&
				amountUnit != ETHER_GWEI &&
				amountUnit != ETHER_ETHER) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid amount unit");
			}

			EthereumAmount::Unit unit = EthereumAmount::Unit(amountUnit);
			nlohmann::json j;
			EthereumTransferPtr tx = _client->_ewm->getWallet()->createTransfer(targetAddress, amount, unit, nonce);

            std::string rawtx = tx->RlpEncode(_client->_ewm->getNetwork()->getRaw(), RLP_TYPE_TRANSACTION_UNSIGNED);

			j["TxUnsigned"] = rawtx;
			j["Fee"] = tx->getFee(unit);
			j["Unit"] = tx->getDefaultUnit();

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::CreateTransferGeneric(const std::string &targetAddress,
																	const std::string &amount,
																	EthereumAmountUnit amountUnit,
																	const std::string &gasPrice,
																	EthereumAmountUnit gasPriceUnit,
																	const std::string &gasLimit,
																	const std::string &data,
																	uint64_t nonce) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("target: {}", targetAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("amountUnit: {}", amountUnit);
			ArgInfo("gasPrice: {}", gasPrice);
			ArgInfo("gasPriceUnit: {}", gasPriceUnit);
			ArgInfo("gasLimit: {}", gasLimit);
			ArgInfo("data: {}", data);
			ArgInfo("nonce: {}", nonce);

			if (amountUnit != TOKEN_DECIMAL &&
				amountUnit != TOKEN_INTEGER &&
				amountUnit != ETHER_WEI &&
				amountUnit != ETHER_GWEI &&
				amountUnit != ETHER_ETHER) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid amount unit");
			}

			EthereumAmount::Unit unit = EthereumAmount::Unit(amountUnit);
			EthereumAmount::Unit gasUnit = EthereumAmount::Unit(gasPriceUnit);
			nlohmann::json j;
			EthereumTransferPtr tx = _client->_ewm->getWallet()->createTransferGeneric(targetAddress,
																					   amount,
																					   unit,
																					   gasPrice,
																					   gasUnit,
																					   gasLimit,
																					   data,
																					   nonce);

            std::string rawtx = tx->RlpEncode(_client->_ewm->getNetwork()->getRaw(), RLP_TYPE_TRANSACTION_UNSIGNED);

			j["TxUnsigned"] = rawtx;
			j["Fee"] = tx->getFee(unit);
			j["Unit"] = tx->getDefaultUnit();

			ArgInfo("r => {}", j.dump());

			return j;
		}

		EthSidechainSubWallet::EthSidechainSubWallet(const CoinInfoPtr &info,
													 const ChainConfigPtr &config,
													 MasterWallet *parent,
													 const std::string &netType) :
													 _info(info),
													 _parent(parent) {

			_walletID = _parent->GetID() + ":" + info->GetChainID();

			AccountPtr account = _parent->GetAccount();
			bytes_t pubkey = account->GetETHSCPubKey();
			if (pubkey.empty()) {
				if (!account->HasMnemonic() || account->Readonly()) {
					ErrorChecker::ThrowParamException(Error::UnsupportOperation, "unsupport operation: ethsc pubkey is empty");
				} else {
					if (account->HasPassphrase()) {
						ErrorChecker::ThrowParamException(Error::Other, "need to call IMasterWallet::VerifyPassPhrase() first");
					} else {
						ErrorChecker::ThrowParamException(Error::Other, "need to call IMasterWallet::VerifyPayPassword() first");
					}
				}
			}

            std::string netName = info->GetChainID() + "-" + netType;
            BREthereumNetwork net = FindEthereumNetwork(netName.c_str());
            EthereumNetworkPtr network(new EthereumNetwork(net));
			_client = ClientPtr(new EthereumClient(network, parent->GetDataPath(), pubkey));
			_client->_ewm->getWallet()->setDefaultGasPrice(5000000000);
		}

		std::string EthSidechainSubWallet::GetChainID() const {
			return _info->GetChainID();
		}

		nlohmann::json EthSidechainSubWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			EthereumWalletPtr wallet = _client->_ewm->getWallet();
			nlohmann::json j, jinfo;

			jinfo["Symbol"] = wallet->getSymbol();
			jinfo["GasLimit"] = wallet->getDefaultGasLimit();
			jinfo["GasPrice"] = wallet->getDefaultGasPrice();
			jinfo["Account"] = wallet->getAccount()->getPrimaryAddress();
			jinfo["HoldsEther"] = wallet->walletHoldsEther();

			j["Info"] = jinfo;
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string EthSidechainSubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletID, GetFunName());

			std::string addr = _client->_ewm->getWallet()->getAccount()->getPrimaryAddress();

			ArgInfo("r => {}", addr);
			return addr;
		}

		nlohmann::json EthSidechainSubWallet::GetAllAddress(uint32_t start, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("internal: {}", internal);

			std::vector<std::string> addresses;
			addresses.push_back(_client->_ewm->getWallet()->getAccount()->getPrimaryAddress());
			nlohmann::json j;
			j["Addresses"] = addresses;
			j["MaxCount"] = 1;

			ArgInfo("r => {}", j.dump());
			return j;
		}

        std::vector<std::string> EthSidechainSubWallet::GetLastAddresses(bool internal) const {
            return {};
		}

        void EthSidechainSubWallet::UpdateUsedAddress(const std::vector<std::string> &usedAddresses) const {

		}

        nlohmann::json EthSidechainSubWallet::GetAllPublicKeys(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("s: {}", start);
			ArgInfo("c: {}", count);

			std::vector<std::string> pubkey;
			pubkey.push_back(_client->_ewm->getWallet()->getAccount()->getPrimaryAddressPublicKey().getHex());
			nlohmann::json j;
			j["PublicKeys"] = pubkey;
			j["MaxCount"] = 1;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::CreateTransaction(const nlohmann::json &inputs,
                                                                const nlohmann::json &outputs,
                                                                const std::string &fee,
                                                                const std::string &memo) {
			ErrorChecker::ThrowParamException(Error::UnsupportOperation, "use IEthSidechainSubWallet::CreateTransfer() instead");
			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::SignTransaction(const nlohmann::json &tx,
															  const std::string &payPassword) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());
			ArgInfo("passwd: *");

			std::string rlptx;
			EthereumTransferPtr transfer;
            EthereumAmount::Unit amountUnit;

			try {
				rlptx = tx["TxUnsigned"].get<std::string>();
                amountUnit = EthereumAmount::Unit(tx["Unit"].get<int>());

                BREthereumTransaction transaction = transactionRlpHexDecode (_client->_ewm->getNetwork()->getRaw(), RLP_TYPE_TRANSACTION_UNSIGNED, rlptx.c_str());
                BREthereumTransfer brtransfer = transferCreateWithTransactionOriginating (
                        transaction,
                        (_client->_ewm->getWallet()->walletHoldsEther()
                         ? TRANSFER_BASIS_TRANSACTION
                         : TRANSFER_BASIS_LOG));
                transfer = EthereumTransferPtr(new EthereumTransfer(_client->_ewm.get(), brtransfer, amountUnit));
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "get 'ID' of json failed");
			}

			uint512 seed = _parent->GetAccount()->GetSeed(payPassword);
			BRKey prvkey = derivePrivateKeyFromSeed(*(UInt512 *)seed.begin(), 0);
			_client->_ewm->getWallet()->signWithPrivateKey(transfer, prvkey);

            std::string rawtx = transfer->RlpEncode(_client->_ewm->getNetwork()->getRaw(), RLP_TYPE_TRANSACTION_SIGNED);
			nlohmann::json j;
			j["Hash"] = transfer->getOriginationTransactionHash();
			j["Fee"] = transfer->getFee(amountUnit);
			j["Unit"] = amountUnit;
            j["TxSigned"] = rawtx;

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

		std::string EthSidechainSubWallet::ConvertToRawTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			ArgInfo("r => ");

			return "";
		}

	}
}