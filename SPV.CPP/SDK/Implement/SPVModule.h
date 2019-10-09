// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVMODULE_H__
#define __ELASTOS_SDK_SPVMODULE_H__

#include "SubWallet.h"

#include <SDK/Account/SideAccount.h>
#include <SDK/Common/uint256.h>
#include <SDK/Database/NotifyQueue.h>
#include <SDK/SpvService/Config.h>

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
					  const PluginType &plugin_types, const ChainParamsPtr &chain_params);

		private:
			// Override SpvService methods.
			void onTxAdded(const TransactionPtr &tx) override;

			void onTxUpdated(const std::vector<uint256> &hashes, uint32_t block_height,
							 time_t timestamp) override;

			void onTxDeleted(const uint256 &hash, bool notify, bool rescan) override;

			void syncProgress(uint32_t current_height, uint32_t best_height,
							  time_t) override;

		private:
			const time_t MINIMUM_NOTIFY_GAP = 100; // 10 seconds.

			NotifyQueue _notify_queue;
			Listener *_listener;

		public:
			static SPVModulePtr Create(const std::string &chain_id,
									   const uint256 &genesis_hash,
									   const std::string &root_path) {
				Config cfg(root_path);
				ChainConfigPtr chain_cfg = cfg.GetChainConfig("ELA");
				ChainParamsPtr chain_params = chain_cfg->ChainParameters();
				std::string db_path = root_path + "/spv_module.db";
				SubAccountPtr sub_account = SubAccountPtr(new SideAccount(genesis_hash));
				return SPVModulePtr(new SPVModule(chain_id, sub_account, db_path,
												  chain_params->LastCheckpoint().Timestamp(),
												  chain_cfg->PluginType(), chain_params));
			}
		};

	} // namespace ElaWallet
} // namespace Elastos

#endif //__ELASTOS_SDK_SPVMODULE_H__
