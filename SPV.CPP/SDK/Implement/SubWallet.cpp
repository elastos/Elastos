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
#include "SubWallet.h"

#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/TransactionInput.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Transaction/Payload/TransferAsset.h>
#include <Account/SubAccount.h>
#include <WalletCore/CoinInfo.h>
#include <Wallet/Wallet.h>
#include <SpvService/Config.h>
#include <Wallet/WalletCommon.h>

#include <algorithm>

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent,
							 const std::string &netType) :
			_parent(parent),
			_info(info),
			_config(config) {

			fs::path subWalletDBPath = _parent->GetDataPath();
			subWalletDBPath /= _info->GetChainID() + DB_FILE_EXTENSION;

			SubAccountPtr subAccount = SubAccountPtr(new SubAccount(_parent->_account));
			_walletManager = WalletManagerPtr(
				new SpvService(_parent->GetID(), _info->GetChainID(), subAccount, subWalletDBPath,
							   _config, netType));

			WalletPtr wallet = _walletManager->GetWallet();
		}

		SubWallet::SubWallet(const std::string &netType,
							 MasterWallet *parent,
							 const ChainConfigPtr &config,
							 const CoinInfoPtr &info) :
			_parent(parent),
			_info(info),
			_config(config) {

		}

		SubWallet::~SubWallet() {
		}

		std::string SubWallet::GetChainID() const {
			return _info->GetChainID();
		}

		const std::string &SubWallet::GetInfoChainID() const {
			return _info->GetChainID();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		std::string SubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::string address = _walletManager->GetWallet()->GetReceiveAddress().String();

			ArgInfo("r => {}", address);

			return address;
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			AddressArray addresses;
			size_t maxCount = _walletManager->GetWallet()->GetAllAddresses(addresses, start, count, internal);

			std::vector<std::string> addrString;
			for (size_t i = 0; i < addresses.size(); ++i) {
				addrString.push_back(addresses[i].String());
			}

			j["Addresses"] = addrString;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

        std::vector<std::string> SubWallet::GetLastAddresses(bool internal) const {
            WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());

            AddressArray addresses = wallet->GetLastAddresses(internal);

            std::vector<std::string> addr;
            for (Address &a : addresses)
                addr.push_back(a.String());

            nlohmann::json j = addr;
            ArgInfo("r => {}", j.dump());

            return addr;
		}

        void SubWallet::UpdateUsedAddress(const std::vector<std::string> &usedAddresses) const {
            nlohmann::json addressJson = usedAddresses;
            WalletPtr wallet = _walletManager->GetWallet();

		    ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
		    ArgInfo("usedAddresses: {}", addressJson.dump());

            AddressSet addressSet;
		    for (const std::string &addr : usedAddresses)
		        addressSet.insert(Address(addr));

		    if (!addressSet.empty())
                wallet->UpdateUsedAddresses(addressSet);
		}

        nlohmann::json SubWallet::GetAllPublicKeys(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			std::vector<bytes_t> publicKeys;
			size_t maxCount = _walletManager->GetWallet()->GetAllPublickeys(publicKeys, start, count, false);

			std::vector<std::string> pubKeyString;
			for (size_t i = 0; i < publicKeys.size(); ++i) {
				pubKeyString.push_back(publicKeys[i].getHex());
			}

			j["PublicKeys"] = pubKeyString;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		void SubWallet::EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const {
			ByteStream stream;
			tx->Serialize(stream, true);
			const bytes_t &hex = stream.GetBytes();

			result["Algorithm"] = "base64";
			result["ID"] = tx->GetHash().GetHex().substr(0, 8);
			result["Data"] = hex.getBase64();
			result["ChainID"] = GetChainID();
			result["Fee"] = tx->GetFee();
		}

		TransactionPtr SubWallet::DecodeTx(const nlohmann::json &encodedTx) const {
			if (encodedTx.find("Algorithm") == encodedTx.end() ||
				encodedTx.find("Data") == encodedTx.end() ||
				encodedTx.find("ChainID") == encodedTx.end()) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid input");
			}

			std::string algorithm, data, chainID;
			uint64_t fee = 0;

			try {
				algorithm = encodedTx["Algorithm"].get<std::string>();
				data = encodedTx["Data"].get<std::string>();
				chainID = encodedTx["ChainID"].get<std::string>();
                if (encodedTx.contains("Fee"))
                    fee = encodedTx["Fee"].get<uint64_t>();
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid input: " + std::string(e.what()));
			}

			if (chainID != GetChainID()) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument,
												  "Invalid input: tx is not belongs to current subwallet");
			}

			TransactionPtr tx;
			if (GetChainID() == CHAINID_MAINCHAIN) {
				tx = TransactionPtr(new Transaction());
			} else if (GetChainID() == CHAINID_IDCHAIN || GetChainID() == CHAINID_TOKENCHAIN) {
				tx = TransactionPtr(new IDTransaction());
			}

			bytes_t rawHex;
			if (algorithm == "base64") {
				rawHex.setBase64(data);
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidArgument, "Decode tx with unknown algorithm");
			}

			ByteStream stream(rawHex);
			ErrorChecker::CheckParam(!tx->Deserialize(stream, true), Error::InvalidArgument,
									 "Invalid input: deserialize fail");
			tx->SetFee(fee);

			SPVLOG_DEBUG("decoded tx: {}", tx->ToJson().dump(4));
			return tx;
		}

        bool SubWallet::UTXOFromJson(UTXOSet &utxo, const nlohmann::json &j) {
            for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
                if (!(*it).contains("TxHash") ||
                    !(*it).contains("Index") ||
                    !(*it).contains("Address") ||
                    !(*it).contains("Amount")) {
                    ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid inputs");
                }

                uint256 hash;
                hash.SetHex((*it)["TxHash"].get<std::string>());
                uint16_t n = (*it)["Index"].get<uint16_t>();

                Address address((*it)["Address"].get<std::string>());
                ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid address of inputs");

                BigInt amount;
                amount.setDec((*it)["Amount"].get<std::string>());
                ErrorChecker::CheckParam(amount < 0, Error::InvalidArgument, "invalid amount of inputs");

                utxo.insert(UTXOPtr(new UTXO(hash, n, address, amount)));
            }
            return true;
		}

        bool SubWallet::OutputsFromJson(OutputArray &outputs, const nlohmann::json &j) {
            for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
                BigInt amount;
                amount.setDec((*it)["Amount"].get<std::string>());
                ErrorChecker::CheckParam(amount < 0, Error::InvalidArgument, "invalid amount of outputs");

                Address address((*it)["Address"].get<std::string>());
                ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid address of outputs");

                OutputPtr output(new TransactionOutput(TransactionOutput(amount, address)));
                outputs.push_back(output);
            }
            return true;
		}

        nlohmann::json SubWallet::CreateTransaction(const nlohmann::json &inputsJson,
                                                    const nlohmann::json &outputsJson,
                                                    const std::string &fee,
                                                    const std::string &memo) {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("outputs: {}", outputsJson.dump());
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxos;
            UTXOFromJson(utxos, inputsJson);

			OutputArray outputs;
			OutputsFromJson(outputs, outputsJson);

			BigInt feeAmount;
			feeAmount.setDec(fee);

			PayloadPtr payload = PayloadPtr(new TransferAsset());
			TransactionPtr tx = wallet->CreateTransaction(Transaction::transferAsset,
														  payload, utxos, outputs, memo, feeAmount);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &tx,
												  const std::string &payPassword) const {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", tx.dump());
			ArgInfo("passwd: *");

			TransactionPtr txn = DecodeTx(tx);

			_walletManager->GetWallet()->SignTransaction(txn, payPassword);

			nlohmann::json result;
			EncodeTx(result, txn);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string SubWallet::ConvertToRawTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", tx.dump());

			TransactionPtr txn = DecodeTx(tx);
			ByteStream stream;
			txn->Serialize(stream, false);
			std::string rawtx = stream.GetBytes().getHex();

			ArgInfo("r => {}", rawtx);

			return rawtx;
		}

		const CoinInfoPtr &SubWallet::GetCoinInfo() const {
			return _info;
		}

		void SubWallet::FlushData() {
			_walletManager->DatabaseFlush();
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			nlohmann::json j;
			j["Info"] = _walletManager->GetWallet()->GetBasicInfo();
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedInfo(const nlohmann::json &encodedTx) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", encodedTx.dump());

			TransactionPtr tx = DecodeTx(encodedTx);

			nlohmann::json info = tx->GetSignedInfo();

			ArgInfo("r => {}", info.dump());

			return info;
		}

	}
}
