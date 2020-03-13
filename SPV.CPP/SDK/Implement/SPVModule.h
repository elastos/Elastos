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
#ifndef __ELASTOS_SDK_SPVMODULE_H__
#define __ELASTOS_SDK_SPVMODULE_H__

#include "SubWallet.h"

#include <Account/SideAccount.h>
#include <Common/uint256.h>
#include <Database/NotifyQueue.h>
#include <SpvService/Config.h>

namespace Elastos {
	namespace ElaWallet {

		class SPVModule;

		typedef boost::shared_ptr<SPVModule> SPVModulePtr;

		/**
		 * SPVModule is a SPV module for side chain usage.  It doesn't store any
		 * private key, and only receive cross chain deposit transactions according
		 * to the registered side chain genesis address.
		 */
		class SPVModule : public SpvService {
		public:
			class Listener {
			public:
				/**
				 * Callback the confirmed(6 blocks) deposit transaction according to the
				 * side chain genesis address.
				 *
				 * @param tx, the confirmed deposit transaction.
				 */
				virtual void OnDepositTxConfirmed(const TransactionPtr &tx) = 0;
			};

			~SPVModule();

			/**
			 * Register a callback for a specified side chain genesis address.
			 *
			 * @param listener, the listener specify the side chain genesis address and
			 * receive callbacks.
			 */
			void RegisterListener(Listener *listener);

			/**
			 * Submit the executed deposit transaction hash, which was notified through
			 * the Listener->OnDepositTxConfirmed() before. So that SPV module knows
			 * the transaction has been executed successfully and stop retry notifying
			 * the specified transaction.
			 *
			 * @param tx_hash, the transaction hash which has been executed successfully.
			 */
			void SubmitTxReceipt(const uint256 &tx_hash);

		protected:
			SPVModule(const std::string &wallet_id, const SubAccountPtr &sub_account,
					  const boost::filesystem::path &db_path, time_t earliest_peer_time,
					  const ChainConfigPtr &config, const std::string &netType);

		private:
			// Override SpvService methods.
			void onTxAdded(const TransactionPtr &tx) override;

			void onTxUpdated(const std::vector<TransactionPtr> &txns) override;

			void onTxDeleted(const TransactionPtr &tx, bool notify, bool rescan) override;

			void syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) override;

		private:
			const time_t MINIMUM_NOTIFY_GAP = 100; // 10 seconds.

			NotifyQueue _notify_queue;
			Listener *_listener;

		public:
			static SPVModulePtr Create(const std::string &genesis_hash, const std::string &root_path) {
				Log::registerMultiLogger(root_path);
				Config cfg(root_path);
				ChainConfigPtr chain_cfg = cfg.GetChainConfig(CHAINID_MAINCHAIN);
				std::string db_path = root_path + "/spv_module.db";
				SubAccountPtr sub_account = SubAccountPtr(new SideAccount(uint256(genesis_hash)));
				return SPVModulePtr(new SPVModule(CHAINID_MAINCHAIN, sub_account, db_path,
												  chain_cfg->ChainParameters()->LastCheckpoint().Timestamp(),
												  chain_cfg,
												  cfg.GetNetType()));
			}
		};

	} // namespace ElaWallet
} // namespace Elastos

#endif //__ELASTOS_SDK_SPVMODULE_H__
