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
#ifndef __ELASTOS_SDK_SUBWALLET_H__
#define __ELASTOS_SDK_SUBWALLET_H__

#include <SpvService/SpvService.h>
#include <Account/SubAccount.h>

#include <ISubWallet.h>

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace Elastos {
	namespace ElaWallet {

#define SELA_PER_ELA 100000000
#define DEPOSIT_OR_WITHDRAW_FEE 10000

		class MasterWallet;

		class Transaction;

		class ChainConfig;

		class CoinInfo;

		typedef boost::shared_ptr<Transaction> TransactionPtr;
		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;
		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

		class SubWallet : public virtual ISubWallet,
						  public Lockable {
		public:
			virtual ~SubWallet();

			virtual void FlushData();

		public: //default implement ISubWallet
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

            virtual nlohmann::json GetAddresses(uint32_t index, uint32_t count, bool internal = false) const;

			virtual nlohmann::json GetPublicKeys(uint32_t index, uint32_t count, bool internal = false) const;

			virtual nlohmann::json SignTransaction(const nlohmann::json &tx, const std::string &passwd) const;

            virtual std::string SignDigest(const std::string &address, const std::string &digest, const std::string &passwd) const;

            virtual bool VerifyDigest(const std::string &publicKey, const std::string &digest, const std::string &signature) const;

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfoPtr &info, const ChainConfigPtr &config, MasterWallet *parent);

            std::string GetSubWalletID() const;

		protected:
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
