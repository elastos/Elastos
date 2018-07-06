// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Core/BRTransaction.h>
#include "BRArray.h"
#include "BRPeerMessages.h"

#include "Peer.h"
#include "TransactionMessage.h"
#include "SDK/Transaction/Transaction.h"
#include "Log.h"
#include "Utils.h"
#include "ELATransaction.h"

namespace Elastos {
	namespace ElaWallet {

		TransactionMessage::TransactionMessage() {

		}

		int TransactionMessage::Accept(
				BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;

			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			ELATransaction *tx = ELATransactionNew();
			Transaction trans(tx, false);

			UInt256 txHash;
			int r = 1;

			if (!trans.Deserialize(stream)) {
				peer_log(peer, "malformed tx message with length: %zu", msgLen);
				r = 0;
				ELATransactionFree(tx);
			} else if (!ctx->sentFilter && !ctx->sentGetdata) {
				peer_log(peer, "got tx message before loading filter");
				r = 0;
				ELATransactionFree(tx);
			} else {
				txHash = trans.getHash();
				peer_log(peer, "got tx: %s", Utils::UInt256ToString(txHash).c_str());

				if (ctx->relayedTx) {
					ctx->relayedTx(ctx->info, (BRTransaction *)tx);
				} else {
					ELATransactionFree(tx);
				}

				if (ctx->currentBlock) { // we're collecting tx messages for a merkleblock
					for (size_t i = array_count(ctx->currentBlockTxHashes); i > 0; i--) {
						if (! UInt256Eq(&txHash, &(ctx->currentBlockTxHashes[i - 1]))) continue;
						array_rm(ctx->currentBlockTxHashes, i - 1);
						break;
					}

					if (array_count(ctx->currentBlockTxHashes) == 0) { // we received the entire block including all matched tx
						BRMerkleBlock *block = ctx->currentBlock;

						ctx->currentBlock = NULL;
						if (ctx->relayedBlock) ctx->relayedBlock(ctx->info, block);
					}
				}
			}

			return r;
		}

		void TransactionMessage::Send(BRPeer *peer, void *serializable) {

			ELATransaction *tx = (ELATransaction *)serializable;
			Transaction transaction(tx, false);
			ByteStream stream;
			transaction.Serialize(stream);
			CMBlock buf = stream.getBuffer();
			peer_log(peer, "Sending tx: tx hash = %s", Utils::UInt256ToString(tx->raw.txHash));
			BRPeerSendMessage(peer, buf, buf.GetSize(), MSG_TX);
		}
	}
}