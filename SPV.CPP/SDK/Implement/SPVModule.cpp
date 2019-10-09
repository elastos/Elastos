// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SPVModule.h"

namespace Elastos {
	namespace ElaWallet {
		SPVModule::SPVModule(const std::string &wallet_id,
							 const SubAccountPtr &sub_account,
							 const boost::filesystem::path &db_path,
							 time_t earliest_peer_time, const PluginType &plugin_types,
							 const ChainParamsPtr &chain_params)
			: SpvService(wallet_id, sub_account, db_path, earliest_peer_time, 0, plugin_types, chain_params),
			  _notify_queue(db_path) {}

		SPVModule::~SPVModule() {}

		void SPVModule::RegisterListener(SPVModule::Listener *listener) {
			_listener = listener;
		}

		void SPVModule::SubmitTxReceipt(const uint256 &tx_hash) {
			_notify_queue.Delete(tx_hash);
			SpvService::onTxDeleted(tx_hash, false, false);
		}

		void SPVModule::onTxAdded(const TransactionPtr &tx) {
			// Ignore non deposit transactions.
			if (tx->GetTransactionType() != Transaction::transferCrossChainAsset) { return; }

			SpvService::onTxAdded(tx); // Call parent.
			_notify_queue.Upsert(NotifyQueue::RecordPtr(new NotifyQueue::Record(tx->GetHash(), tx->GetBlockHeight())));
		}

		void SPVModule::onTxUpdated(const std::vector<uint256> &hashes, uint32_t block_height, time_t timestamp) {
			SpvService::onTxUpdated(hashes, block_height, timestamp); // Call parent.

			for (auto hash : hashes) {
				if (GetTransaction(hash))
					_notify_queue.Upsert(NotifyQueue::RecordPtr(new NotifyQueue::Record(hash, block_height)));
			}
		}

		void SPVModule::onTxDeleted(const uint256 &hash, bool notify, bool rescan) {
			SpvService::onTxDeleted(hash, notify, rescan); // Call parent.
			_notify_queue.Delete(hash);
		}

		void SPVModule::syncProgress(uint32_t current_height, uint32_t best_height, time_t block_time) {
			SpvService::syncProgress(current_height, best_height, block_time);

			// Get all confirmed transaction records.
			NotifyQueue::Records records = _notify_queue.GetAllConfirmed(current_height);

			// Notify the confirmed transactions.
			for (auto record : records) {
				time_t now = time(NULL);

				// Ignore recent notify.
				if (record->last_notify_time + MINIMUM_NOTIFY_GAP > now)
					continue;

				// Update last notify time.
				_notify_queue.Upsert(NotifyQueue::RecordPtr(
					new NotifyQueue::Record(record->tx_hash, record->height, now)));

				// Notify the confirmed transactions through the registered listener.
				TransactionPtr tx = GetTransaction(record->tx_hash);
				if (_listener && tx)
					_listener->OnDepositTxConfirmed(tx);
			}
		}

	} // namespace ElaWallet
} // namespace Elastos
