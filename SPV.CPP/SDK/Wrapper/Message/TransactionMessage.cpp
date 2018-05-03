// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRArray.h"
#include "BRPeerMessages.h"

#include "TransactionMessage.h"
#include "Transaction.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace SDK {

		TransactionMessage::TransactionMessage() {

		}

		int TransactionMessage::Accept(
				BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *)peer;
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			Transaction wrappedTx;
			wrappedTx.Deserialize(stream);

			BRTransaction *tx = wrappedTx.convertToRaw();
			UInt256 txHash;
			int r = 1;

			if (! tx) {
				Log::getLogger()->warn("malformed tx message with length: {}", msgLen);
				r = 0;
			}
			else if (! ctx->sentFilter && ! ctx->sentGetdata) {
				Log::warn("got tx message before loading filter");
				BRTransactionFree(tx);
				r = 0;
			}
			else {
				txHash = tx->txHash;
				Log::getLogger()->warn("got tx: {}", Utils::UInt256ToString(txHash));

				if (ctx->relayedTx) {
					ctx->relayedTx(ctx->info, tx);
				}
				else BRTransactionFree(tx);

				if (ctx->currentBlock) { // we're collecting tx messages for a merkleblock
					for (size_t i = array_count(ctx->currentBlockTxHashes); i > 0; i--) {
						if (! UInt256Eq(&txHash, &(ctx->currentBlockTxHashes[i - 1]))) continue;
						array_rm(ctx->currentBlockTxHashes, i - 1);
						break;
					}

					if (array_count(ctx->currentBlockTxHashes) == 0) { // we received the entire block including all matched tx
						BRMerkleBlock *block = ctx->currentBlock;

						ctx->currentBlock = nullptr;
						if (ctx->relayedBlock) ctx->relayedBlock(ctx->info, block);
					}
				}
			}

			return r;
		}

		void TransactionMessage::Send(BRPeer *peer, void *serializable) {

			BRTransaction *tx = (BRTransaction *)serializable;
			Transaction transaction(BRTransactionCopy(tx));
			ByteStream stream;
			transaction.Serialize(stream);
			BRPeerSendMessage(peer, stream.getBuf(), stream.length(), MSG_TX);
		}
	}
}