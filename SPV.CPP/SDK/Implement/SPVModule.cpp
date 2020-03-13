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
#include "SPVModule.h"

namespace Elastos {
	namespace ElaWallet {
		SPVModule::SPVModule(const std::string &wallet_id,
							 const SubAccountPtr &sub_account,
							 const boost::filesystem::path &db_path,
							 time_t earliest_peer_time,
							 const ChainConfigPtr &config,
							 const std::string &netType)
			: SpvService("", wallet_id, sub_account, db_path, earliest_peer_time, config, netType),
			  _notify_queue(db_path) {}

		SPVModule::~SPVModule() {}

		void SPVModule::RegisterListener(SPVModule::Listener *listener) {
			_listener = listener;
		}

		void SPVModule::SubmitTxReceipt(const uint256 &tx_hash) {
			_notify_queue.Delete(tx_hash);
			DeleteTxn(tx_hash);
		}

		void SPVModule::onTxAdded(const TransactionPtr &tx) {
			// Ignore non deposit transactions.
			if (tx->GetTransactionType() != Transaction::transferCrossChainAsset) { return; }

			SpvService::onTxAdded(tx); // Call parent.
			_notify_queue.Upsert(NotifyQueue::RecordPtr(new NotifyQueue::Record(tx->GetHash(), tx->GetBlockHeight())));
		}

		void SPVModule::onTxUpdated(const std::vector<TransactionPtr> &txns) {
			SpvService::onTxUpdated(txns); // Call parent.

			for (auto tx : txns) {
				if (GetTransaction(tx->GetHash(), CHAINID_MAINCHAIN))
					_notify_queue.Upsert(NotifyQueue::RecordPtr(new NotifyQueue::Record(tx->GetHash(), tx->GetBlockHeight())));
			}
		}

		void SPVModule::onTxDeleted(const TransactionPtr &tx, bool notify, bool rescan) {
			SpvService::onTxDeleted(tx, rescan, false); // Call parent.
			_notify_queue.Delete(tx->GetHash());
		}

		void SPVModule::syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) {
			SpvService::syncProgress(progress, lastBlockTime, bytesPerSecond, downloadPeer);

			uint32_t current_height = GetPeerManager()->GetLastBlockHeight();
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
				TransactionPtr tx = GetTransaction(record->tx_hash, CHAINID_MAINCHAIN);
				if (_listener && tx)
					_listener->OnDepositTxConfirmed(tx);
			}
		}

	} // namespace ElaWallet
} // namespace Elastos
