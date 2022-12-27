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
#include <WalletCore/CoinInfo.h>
#include <SpvService/Config.h>

#include <algorithm>

namespace Elastos {
	namespace ElaWallet {

#define WarnLog() SPVLOG_WARN("SubWallet::{} should not be here", GetFunName())

		SubWallet::SubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent) :
			_parent(parent),
			_info(info),
			_config(config) {
		}

		SubWallet::~SubWallet() {
		}

        void SubWallet::FlushData() {
		    WarnLog();
        }

        //default implement ISubWallet
		std::string SubWallet::GetChainID() const {
			return _info->GetChainID();
		}

        nlohmann::json SubWallet::GetBasicInfo() const {
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());

            nlohmann::json j;
            j["Info"] = {};
            j["ChainID"] = _info->GetChainID();

            ArgInfo("r => {}", j.dump());
            return j;
        }

        nlohmann::json SubWallet::GetAddresses(uint32_t index, uint32_t count, bool internal) const {
            WarnLog();
            return nlohmann::json();
		}

        nlohmann::json SubWallet::GetPublicKeys(uint32_t index, uint32_t count, bool internal) const {
            WarnLog();
			return nlohmann::json();
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &tx, const std::string &passwd) const {
		    WarnLog();
			return nlohmann::json();
		}

        std::string SubWallet::SignDigest(const std::string &address, const std::string &digest, const std::string &passwd) const {
            WarnLog();
            return "";
		}

        bool SubWallet::VerifyDigest(const std::string &publicKey, const std::string &digest, const std::string &signature) const {
            WarnLog();
		    return false;
		}

        std::string SubWallet::GetSubWalletID() const {
            return _parent->GetWalletID() + ":" + _info->GetChainID();
        }

	}
}
