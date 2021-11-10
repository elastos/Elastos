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

#include "BTCSubWallet.h"
#include "MasterWallet.h"
#include <WalletCore/CoinInfo.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Base58.h>
#include <bitcoin/BRTransaction.h>
#include <ethereum/util/BRUtil.h>


namespace Elastos {
    namespace ElaWallet {

        BTCSubWallet::~BTCSubWallet() {

        }

        BTCSubWallet::BTCSubWallet(const CoinInfoPtr &info,
                                   const ChainConfigPtr &config,
                                   MasterWallet *parent,
                                   const std::string &netType) :
                                   SubWallet(info, config, parent) {
            AccountPtr account = _parent->GetAccount();

            if (account->BitcoinMasterPubKey() == nullptr)
                ErrorChecker::ThrowParamException(Error::CreateSubWalletError, "need to call IMasterWallet::VerifyPayPassword() or re-import wallet first");

            if (netType == CONFIG_MAINNET) {
                _addrParams = BITCOIN_ADDRESS_PARAMS;
            } else {
                _addrParams = BITCOIN_TEST_ADDRESS_PARAMS;
            }
        }

        nlohmann::json BTCSubWallet::GetAddresses(uint32_t index, uint32_t count, bool internal) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("index: {}", index);
            ArgInfo("count: {}", count);
            ArgInfo("internal: {}", internal);

            ErrorChecker::CheckParam(index + count <= index, Error::InvalidArgument, "index + count overflow");
            nlohmann::json addrJson;

            std::vector<uint160> addresses;
            GetAddressInternal(addresses, index, count, internal);

            for (uint160 &addr : addresses) {
                bytes_t bytes = addr.bytes();
                BRAddress braddr;
                memset(braddr.s, 0, sizeof(braddr));
                BRAddressFromHash160(braddr.s, sizeof(braddr), _addrParams, bytes.data());
                addrJson.push_back(std::string(braddr.s));
            }

            ArgInfo("r => {}", addrJson.dump(4));
            return addrJson;
        }

        nlohmann::json BTCSubWallet::GetPublicKeys(uint32_t index, uint32_t count, bool internal) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("index: {}", index);
            ArgInfo("count: {}", count);
            ArgInfo("internal: {}", internal);
            nlohmann::json pubkeyJson;
            AccountPtr account = _parent->GetAccount();

            ErrorChecker::CheckParam(index + count <= index, Error::InvalidArgument, "index + count overflow");
            if (account->SingleAddress()) {
                index = 0;
                count = 1;
                internal = false;
            }

            uint32_t chain = internal ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;

            if (account->GetSignType() == Account::MultiSign) {
                Log::error("unsupport btc multi-sign wallet");
            } else {
                HDKeychain keychain = account->BitcoinMasterPubKey()->getChild(chain);
                for (uint32_t i = index; i < index + count; ++i)
                    pubkeyJson.push_back(keychain.getChild(i).pubkey().getHex());
            }

            ArgInfo("r => {}", pubkeyJson.dump(4));
            return pubkeyJson;
        }

        nlohmann::json BTCSubWallet::CreateTransaction(
                const nlohmann::json &inputs,
                const nlohmann::json &outputs,
                const std::string &changeAddress,
                const std::string &feePerKB) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputs.dump());
            ArgInfo("outputs: {}", outputs.dump());
            ArgInfo("changeAddress: {}", changeAddress);
            ArgInfo("feePerKB: {}", feePerKB);

            ErrorChecker::CheckParam(!inputs.is_array(), Error::InvalidArgument, "inputs it not array");
            ErrorChecker::CheckParam(!outputs.is_array(), Error::InvalidArgument, "outputs it not array");

            BRTransaction *tx = BRTransactionNew();
            try {
                uint64_t outputAmount = 0, inputAmount = 0, feeAmount = 0, feeRate = 0, minAmount = 0;
                size_t outCount = 0;
                feeRate = strtoll(feePerKB.c_str(), NULL, 10);

                for (nlohmann::json::const_iterator it = inputs.cbegin(); it != inputs.cend(); ++it) {
                    BRTxOutput o = BR_TX_OUTPUT_NONE;
                    uint256 h((*it)["TxHash"].get<std::string>());
                    UInt256 txHash = UInt256Get(h.begin());
                    uint16_t index = (*it)["Index"].get<uint16_t>();
                    std::string addr = (*it)["Address"].get<std::string>();
                    ErrorChecker::CheckParam(!BRAddressIsValid(_addrParams, addr.c_str()),
                                             Error::InvalidArgument, "invalid input address");
                    o.amount = strtoll((*it)["Amount"].get<std::string>().c_str(), NULL, 10);

                    inputAmount += o.amount;
                    BRTxOutputSetAddress(&o, _addrParams, addr.c_str());
                    BRTransactionAddInput(tx, txHash, index, o.amount, o.script, o.scriptLen, NULL, 0, NULL, 0, TXIN_SEQUENCE);
                    BRTxOutputSetAddress(&o, _addrParams, NULL);

                    ErrorChecker::CheckParam((BRTransactionVSize(tx) + TX_OUTPUT_SIZE > TX_MAX_SIZE),
                                             Error::InvalidArgument, "tx too large");
                }

                for (nlohmann::json::const_iterator it = outputs.cbegin(); it != outputs.cend(); ++it) {
                    BRTxOutput o = BR_TX_OUTPUT_NONE;
                    std::string addr = (*it)["Address"].get<std::string>();
                    o.amount = strtoll((*it)["Amount"].get<std::string>().c_str(), NULL, 10);
                    ErrorChecker::CheckParam(!BRAddressIsValid(_addrParams, addr.c_str()),
                                             Error::InvalidArgument, "invalid output address");
                    outputAmount += o.amount;
                    BRTxOutputSetAddress(&o, _addrParams, addr.c_str());
                    BRTransactionAddOutput(tx, o.amount, o.script, o.scriptLen);
                    BRTxOutputSetAddress(&o, _addrParams, NULL);
                    outCount++;
                }
                ErrorChecker::CheckParam(outCount < 1, Error::InvalidArgument, "no outputs");

                minAmount = MinOutputAmountWithFeePerKb(feeRate);
                feeAmount = TxFee(feeRate, BRTransactionVSize(tx) + TX_OUTPUT_SIZE);
                if (inputAmount < outputAmount + feeAmount) {
                    ErrorChecker::ThrowParamException(Error::InvalidArgument, "insufficient funds");
                } else if (inputAmount - (outputAmount + feeAmount) > minAmount) {
                    BRTxOutput o = BR_TX_OUTPUT_NONE;
                    ErrorChecker::CheckParam(!BRAddressIsValid(_addrParams, changeAddress.c_str()),
                                             Error::InvalidArgument, "invalid change address");
                    o.amount = inputAmount - (outputAmount + feeAmount);
                    BRTxOutputSetAddress(&o, _addrParams, changeAddress.c_str());
                    BRTransactionAddOutput(tx, o.amount, o.script, o.scriptLen);
                    BRTxOutputSetAddress(&o, _addrParams, NULL);
                    BRTransactionShuffleOutputs(tx);
                }
            } catch (const std::exception &e) {
                BRTransactionFree(tx);
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "create tx failed: " + std::string(e.what()));
            }

            size_t txSize = BRTransactionSerialize(tx, NULL, 0);
            bytes_t txbuf(txSize);
            BRTransactionSerialize(tx, txbuf.data(), txbuf.size());

            uint256 txHash;
            memcpy(txHash.begin(), tx->txHash.u8, txHash.size());

            nlohmann::json txJson;
            txJson["Data"] = txbuf.getHex();
            txJson["TxHash"] = txHash.GetHex();

            ArgInfo("r => {}", txJson.dump(4));
            return txJson;
        }

        nlohmann::json BTCSubWallet::SignTransaction(const nlohmann::json &txJson, const std::string &passwd) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("tx: {}", txJson.dump(4));
            ArgInfo("passwd: *");

            nlohmann::json txSigned;
            try {
                bytes_t txData;
                txData.setHex(txJson["Data"].get<std::string>());
                BRTransaction *tx = BRTransactionParse(txData.data(), txData.size());
                ErrorChecker::CheckParam(tx == NULL, Error::InvalidArgument, "deserialize tx from json failed");

                // find the keys
                std::vector<uint160> pkhs;
                for (size_t i = 0; i < tx->inCount; ++i) {
                    const uint8_t *pkh = BRScriptPKH(tx->inputs[i].script, tx->inputs[i].scriptLen);
                    pkhs.push_back(uint160(bytes_t(pkh, 20)));
                }

                std::vector<BRKey> brkeys = FindBRKeys(pkhs, passwd);

                int r = BRTransactionSign(tx, 0, brkeys.data(), brkeys.size());
                ErrorChecker::CheckLogic(r == 0, Error::Sign, "sign btc tx failed");

                for (size_t i = 0; i < brkeys.size(); i++)
                    BRKeyClean(&brkeys[i]);

                size_t txSize = BRTransactionSerialize(tx, NULL, 0);
                bytes_t txbuf(txSize);
                BRTransactionSerialize(tx, txbuf.data(), txbuf.size());

                uint256 txHash;
                memcpy(txHash.begin(), tx->txHash.u8, txHash.size());

                txSigned["Data"] = txbuf.getHex();
                txSigned["TxHash"] = txHash.GetHex();
            } catch (const nlohmann::detail::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "parse tx json failed");
            }

            ArgInfo("r => {}", txSigned.dump(4));
            return txSigned;
        }

        std::string BTCSubWallet::SignDigest(const std::string &address, const std::string &digest, const std::string &passwd) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("address: {}", address);
            ArgInfo("digest: {}", digest);
            ArgInfo("passwd: *");

            std::vector<uint160> pkhs;
            BRTxOutput o = BR_TX_OUTPUT_NONE;
            BRTxOutputSetAddress(&o, _addrParams, address.c_str());
            const uint8_t *pkh = BRScriptPKH(o.script, o.scriptLen);
            pkhs.push_back(uint160(bytes_t(pkh, 20)));

            std::vector<HDKeychain> keys = FindKeys(pkhs, passwd);
            ErrorChecker::CheckParam(keys.empty(), Error::InvalidArgument, "key not found");

            Key k = keys[0];
            std::string sig = k.SignDER(uint256(digest)).getHex();

            ArgInfo("r => {}", sig);

            return sig;
        }

        bool BTCSubWallet::VerifyDigest(const std::string &pubkey, const std::string &digest, const std::string &signature) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("pubkey: {}", pubkey);
            ArgInfo("digest: {}", digest);
            ArgInfo("signature: {}", signature);

            Key k(CTBitcoin, pubkey);
            bool r = k.VerifyDER(uint256(digest), signature);

            ArgInfo("r => {}", r);

            return r;
        }

        void BTCSubWallet::FlushData() {

        }

        nlohmann::json BTCSubWallet::GetLegacyAddresses(uint32_t index, uint32_t count, bool internal) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("index: {}", index);
            ArgInfo("count: {}", count);
            ArgInfo("internal: {}", internal);
            nlohmann::json addrJson;

            ErrorChecker::CheckParam(index + count <= index, Error::InvalidArgument, "index + count overflow");
            std::vector<uint160> addresses;
            GetAddressInternal(addresses, index, count, internal);

            for (uint160 &addr : addresses) {
                bytes_t bytes = addr.bytes();
                bytes.insert(bytes.begin(), _addrParams.pubKeyPrefix);

                addrJson.push_back(Base58::CheckEncode(bytes));
            }

            ArgInfo("r => {}", addrJson.dump(4));
            return addrJson;
        }

        void BTCSubWallet::GetAddressInternal(std::vector<uint160> &chainAddresses, uint32_t index, uint32_t count, bool internal) const {
            AccountPtr account = _parent->GetAccount();

            if (account->SingleAddress()) {
                index = 0;
                count = 1;
                internal = false;
            }

            uint32_t chain = internal ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;
            std::vector<uint160> &addrChain = _chainAddressCached[chain];
            uint32_t derivateCount = (index + count > addrChain.size()) ? (index + count - addrChain.size()) : 0;

            if (account->GetSignType() == Account::MultiSign) {
                Log::error("unsupport btc multi-sign wallet");
            } else {
                HDKeychain keychain = account->BitcoinMasterPubKey()->getChild(chain);

                while (derivateCount--) {
                    addrChain.emplace_back(hash160(keychain.getChild(addrChain.size()).pubkey()));
                }

                if (index + count <= addrChain.size())
                    chainAddresses.assign(addrChain.begin() + index, addrChain.begin() + index + count);
            }
        }

        uint64_t BTCSubWallet::MinOutputAmountWithFeePerKb(uint64_t feePerKb) const {
            uint64_t amount = (TX_MIN_OUTPUT_AMOUNT*feePerKb + MIN_FEE_PER_KB - 1)/MIN_FEE_PER_KB;
            return (amount > TX_MIN_OUTPUT_AMOUNT) ? amount : TX_MIN_OUTPUT_AMOUNT;
        }

        uint64_t BTCSubWallet::TxFee(uint64_t feePerKb, size_t size) const {
            uint64_t standardFee = size*TX_FEE_PER_KB/1000,       // standard fee based on tx size
            fee = (((size*feePerKb/1000) + 99)/100)*100; // fee using feePerKb, rounded up to nearest 100 satoshi

            return (fee > standardFee) ? fee : standardFee;
        }

        std::vector<HDKeychain> BTCSubWallet::FindKeys(const std::vector<uint160> &pkhs, const std::string &passwd) const {
            // find the keys
            std::vector<uint32_t> internalIdx, externalIdx;
            for (size_t i = 0; i < pkhs.size(); ++i) {
                bool found = false;

                for (uint32_t idx = (uint32_t)_chainAddressCached[SEQUENCE_INTERNAL_CHAIN].size(); idx > 0 && !found; idx--) {
                    if (pkhs[i] == _chainAddressCached[SEQUENCE_INTERNAL_CHAIN][idx - 1]) {
                        internalIdx.push_back(idx - 1);
                        found = true;
                    }
                }

                for (uint32_t idx = (uint32_t)_chainAddressCached[SEQUENCE_EXTERNAL_CHAIN].size(); idx > 0 && !found; idx--) {
                    if (pkhs[i] == _chainAddressCached[SEQUENCE_EXTERNAL_CHAIN][idx - 1]) {
                        externalIdx.push_back(idx - 1);
                        found = true;
                    }
                }

                ErrorChecker::CheckLogic(!found, Error::PrivateKeyNotFound, "private key not found for index");
            }

            uint512 seed = _parent->GetAccount()->GetSeed(passwd);
            HDKeychain masterKey = HDKeychain(CTBitcoin, HDSeed(seed.bytes()).getExtendedKey(CTBitcoin, true)).getChild("44'/0'/0'");
            seed = 0;

            std::vector<HDKeychain> keys;
            // sign the tx with keys
            for (size_t i = 0; i < internalIdx.size(); ++i) {
                HDKeychain k = masterKey.getChild(SEQUENCE_INTERNAL_CHAIN).getChild(internalIdx[i]);
                keys.push_back(k);
            }
            for (size_t i = 0; i < externalIdx.size(); ++i) {
                HDKeychain k = masterKey.getChild(SEQUENCE_EXTERNAL_CHAIN).getChild(externalIdx[i]);
                keys.push_back(k);
            }

            return keys;
        }

        std::vector<BRKey> BTCSubWallet::FindBRKeys(const std::vector<uint160> &pkhs, const std::string &passwd) const {
            std::vector<HDKeychain> keys = FindKeys(pkhs, passwd);
            std::vector<BRKey> brkeys;
            for (size_t i = 0; i < keys.size(); ++i) {
                BRKey brkey;
                BRKeySetSecret(&brkey, (UInt256*) keys[i].privkey().data(), 1);
                BRKeyPubKey(&brkey, NULL, 0);
                brkeys.push_back(brkey);
            }
            return brkeys;
        }

    }
}