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
#include <ISubWalletCallback.h>

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
			typedef boost::shared_ptr<SpvService> WalletManagerPtr;

			virtual ~SubWallet();

			const WalletManagerPtr &GetWalletManager() const;

			virtual void FlushData();

			virtual const std::string &GetInfoChainID() const;

		public: //implement ISubWallet
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start, uint32_t count, bool internal = false) const;

            virtual std::vector<std::string> GetLastAddresses(bool internal) const;

            virtual void UpdateUsedAddress(const std::vector<std::string> &usedAddresses) const;

			virtual nlohmann::json GetAllPublicKeys(uint32_t start, uint32_t count) const;

			virtual nlohmann::json CreateTransaction(
				const nlohmann::json &inputsJson,
				const nlohmann::json &outputsJson,
				const std::string &fee,
				const std::string &memo);

			virtual nlohmann::json SignTransaction(
				const nlohmann::json &tx,
				const std::string &payPassword) const;

			virtual nlohmann::json GetTransactionSignedInfo(
				const nlohmann::json &rawTransaction) const;

			virtual std::string ConvertToRawTransaction(const nlohmann::json &tx);

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfoPtr &info,
					  const ChainConfigPtr &config,
					  MasterWallet *parent,
					  const std::string &netType);

			SubWallet(const std::string &netType,
					  MasterWallet *parent,
					  const ChainConfigPtr &config,
					  const CoinInfoPtr &info);

			virtual void fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													  const nlohmann::json &desc, uint32_t confirms);

			const CoinInfoPtr &GetCoinInfo() const;

			void EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const;

			TransactionPtr DecodeTx(const nlohmann::json &encodedTx) const;

			bool UTXOFromJson(UTXOSet &utxo, const nlohmann::json &j);

			bool OutputsFromJson(OutputArray &outputs, const nlohmann::json &j);

		protected:
			WalletManagerPtr _walletManager;
			ISubWalletCallback *_callback;
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
