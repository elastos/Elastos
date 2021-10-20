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

        std::string BTCSubWallet::GetChainID() const {
            return _info->GetChainID();
        }

        nlohmann::json BTCSubWallet::GetBasicInfo() const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());

            nlohmann::json j;
            j["Info"] = {};
            j["ChainID"] = _info->GetChainID();

            ArgInfo("r => {}", j.dump());
            return j;
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
                const std::string &fee,
                const std::string &memo) {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());

            return nlohmann::json();
        }

        nlohmann::json BTCSubWallet::SignTransaction(const nlohmann::json &tx, const std::string &payPassword) const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("tx: {}", tx.dump(4));
            ArgInfo("payPassword: *");
            nlohmann::json txSigned;

            ArgInfo("r => {}", txSigned.dump(4));
            return txSigned;
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

    }
}