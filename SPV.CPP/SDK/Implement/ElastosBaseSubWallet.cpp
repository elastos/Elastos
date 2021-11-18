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

#include "ElastosBaseSubWallet.h"
#include "MasterWallet.h"
#include "Account/Account.h"
#include <WalletCore/CoinInfo.h>
#include <Common/Log.h>
#include <Plugin/Transaction/Payload/TransferAsset.h>
#include <Common/ErrorChecker.h>
#include <Wallet/WalletCommon.h>
#include <Plugin/Transaction/IDTransaction.h>

namespace Elastos {
    namespace ElaWallet {

        ElastosBaseSubWallet::ElastosBaseSubWallet(
                const CoinInfoPtr &info,
                const ChainConfigPtr &config,
                MasterWallet *parent,
                const std::string &netType) :
                SubWallet(info, config, parent) {

            ErrorChecker::CheckParam(_parent->GetAccount()->MasterPubKeyHDPMString().empty(), Error::UnsupportOperation, "unsupport to create elastos based wallet");
            boost::filesystem::path subWalletDBPath = _parent->GetDataPath();
            subWalletDBPath /= _info->GetChainID() + ".db";

            SubAccountPtr subAccount = SubAccountPtr(new SubAccount(_parent->GetAccount()));
            _walletManager = WalletManagerPtr(
                    new SpvService(_parent->GetID(), _info->GetChainID(), subAccount, subWalletDBPath,
                                   _config, netType));
        }

        const WalletManagerPtr &ElastosBaseSubWallet::GetWalletManager() const {
            return _walletManager;
        }

        void ElastosBaseSubWallet::FlushData() {
            _walletManager->DatabaseFlush();
        }

        //default implement ISubWallet
        nlohmann::json ElastosBaseSubWallet::GetBasicInfo() const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());

            nlohmann::json j;
            j["Info"] = _walletManager->GetWallet()->GetBasicInfo();
            j["ChainID"] = _info->GetChainID();

            ArgInfo("r => {}", j.dump());
            return j;
        }

        nlohmann::json ElastosBaseSubWallet::GetAddresses(uint32_t index, uint32_t count, bool internal) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("index: {}", index);
            ArgInfo("count: {}", count);
            ArgInfo("internal: {}", internal);

            ErrorChecker::CheckParam(index + count <= index, Error::InvalidArgument, "index & count overflow");

            AddressArray addresses;
            _walletManager->GetWallet()->GetAddresses(addresses, index, count, internal);

            nlohmann::json j;
            for (Address &address : addresses)
                j.push_back(address.String());

            ArgInfo("r => {}", j.dump());

            return j;
        }

        nlohmann::json ElastosBaseSubWallet::GetPublicKeys(uint32_t index, uint32_t count, bool internal) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("index: {}", index);
            ArgInfo("count: {}", count);
            ArgInfo("internal: {}", internal);

            ErrorChecker::CheckParam(index + count <= index, Error::InvalidArgument, "index & count overflow");

            nlohmann::json j;
            _walletManager->GetWallet()->GetPublickeys(j, index, count, internal);

            ArgInfo("r => {}", j.dump());
            return j;
        }

        nlohmann::json ElastosBaseSubWallet::CreateTransaction(const nlohmann::json &inputsJson,
                                                    const nlohmann::json &outputsJson,
                                                    const std::string &fee,
                                                    const std::string &memo) {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("outputs: {}", outputsJson.dump());
            ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            WalletPtr wallet = _walletManager->GetWallet();

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

        nlohmann::json ElastosBaseSubWallet::SignTransaction(const nlohmann::json &tx,
                                                  const std::string &payPassword) const {

            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("tx: {}", tx.dump());
            ArgInfo("passwd: *");

            TransactionPtr txn = DecodeTx(tx);

            _walletManager->GetWallet()->SignTransaction(txn, payPassword);

            nlohmann::json result;
            EncodeTx(result, txn);

            ArgInfo("r => {}", result.dump());
            return result;
        }

        std::string ElastosBaseSubWallet::SignDigest(const std::string &address, const std::string &digest, const std::string &payPassword) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("address: {}", address);
            ArgInfo("digest: {}", digest);
            ArgInfo("payPasswd: *");

            ErrorChecker::CheckParam(digest.size() != 64, Error::InvalidArgument, "invalid digest");
            Address didAddress(address);
            std::string signature = _walletManager->GetWallet()->SignDigestWithAddress(didAddress, uint256(digest), payPassword);

            ArgInfo("r => {}", signature);

            return signature;
        }

        bool ElastosBaseSubWallet::VerifyDigest(const std::string &publicKey, const std::string &digest, const std::string &signature) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("publicKey: {}", publicKey);
            ArgInfo("digest: {}", digest);
            ArgInfo("signature: {}", signature);

            Key k(CTElastos, bytes_t(publicKey));
            bool r = k.Verify(uint256(digest), bytes_t(signature));

            ArgInfo("r => {}", r);
            return r;
        }

        nlohmann::json ElastosBaseSubWallet::GetTransactionSignedInfo(const nlohmann::json &encodedTx) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("tx: {}", encodedTx.dump());

            TransactionPtr tx = DecodeTx(encodedTx);

            nlohmann::json info = tx->GetSignedInfo();

            ArgInfo("r => {}", info.dump());

            return info;
        }

        std::string ElastosBaseSubWallet::ConvertToRawTransaction(const nlohmann::json &tx) {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("tx: {}", tx.dump());

            TransactionPtr txn = DecodeTx(tx);
            ByteStream stream;
            txn->Serialize(stream);
            std::string rawtx = stream.GetBytes().getHex();

            ArgInfo("r => {}", rawtx);

            return rawtx;
        }

        void ElastosBaseSubWallet::EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const {
            ByteStream stream;
            tx->Serialize(stream);
            const bytes_t &hex = stream.GetBytes();

            result["Algorithm"] = "base64";
            result["ID"] = tx->GetHash().GetHex().substr(0, 8);
            result["Data"] = hex.getBase64();
            result["ChainID"] = GetChainID();
            result["Fee"] = tx->GetFee();
        }

        TransactionPtr ElastosBaseSubWallet::DecodeTx(const nlohmann::json &encodedTx) const {
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
            ErrorChecker::CheckParam(!tx->Deserialize(stream), Error::InvalidArgument,
                                     "Invalid input: deserialize fail");
            tx->SetFee(fee);

            SPVLOG_DEBUG("decoded tx: {}", tx->ToJson().dump(4));
            return tx;
        }

        bool ElastosBaseSubWallet::UTXOFromJson(UTXOSet &utxo, const nlohmann::json &j) const {
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

        bool ElastosBaseSubWallet::OutputsFromJson(OutputArray &outputs, const nlohmann::json &j) {
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

    }
}