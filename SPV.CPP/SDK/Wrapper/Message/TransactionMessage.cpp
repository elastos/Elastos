// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRArray.h"
#include "BRPeerMessages.h"

#include "Peer.h"
#include "TransactionMessage.h"
#include "Transaction.h"
#include "Log.h"
#include "Utils.h"
#include "ELACoreExt/ELAPeerContext.h"
#include "ELABRTransaction.h"

namespace Elastos {
	namespace SDK {

		TransactionMessage::TransactionMessage() {

		}

		int TransactionMessage::Accept(
				BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;
			ELAPeerContext *elactx = (ELAPeerContext *) peer;

			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			Transaction wrappedTx;
			wrappedTx.Deserialize(stream);

			BRTransaction *tx = wrappedTx.convertToRaw();
			UInt256 txHash;
			int r = 1;

			if (!tx) {
				Log::getLogger()->warn("malformed tx message with length: {}", msgLen);
				r = 0;
			} else if (!ctx->sentFilter && !ctx->sentGetdata) {
				Log::warn("got tx message before loading filter");
				BRTransactionFree(tx);
				r = 0;
			} else {
				txHash = wrappedTx.getHash();
				Log::getLogger()->info("got tx: {}", Utils::UInt256ToString(txHash));

				if (ctx->relayedTx) {
					ctx->relayedTx(ctx->info, tx);
				} else BRTransactionFree(tx);

				if (elactx->txBlockMap.find(txHash) !=
					elactx->txBlockMap.end()) { // we're collecting tx messages for a merkleblock
					BRMerkleBlock *block = elactx->txBlockMap[txHash];
					std::vector<UInt256> txHashes = elactx->blockTxListMap[block];
					for (size_t i = txHashes.size(); i > 0; i--) {
						if (!UInt256Eq(&txHash, &(txHashes[i - 1]))) continue;
						txHashes.erase(txHashes.begin() + (i - 1));
						break;
					}
					elactx->txBlockMap.erase(txHash);

					if (txHashes.empty()) { // we received the entire block including all matched tx

						elactx->blockTxListMap.erase(block);
						if (ctx->relayedBlock) ctx->relayedBlock(ctx->info, block);
					} else {
						elactx->blockTxListMap[block] = txHashes;
					}
				}
			}

			return r;
		}

		void TransactionMessage::Send(BRPeer *peer, void *serializable) {

			ELABRTransaction *tx = (ELABRTransaction *)serializable;
			Transaction transaction((BRTransaction *)ELABRTransactioCopy(tx));
			ByteStream stream;
			transaction.Serialize(stream);
			BRPeerSendMessage(peer, stream.getBuf(), stream.length(), MSG_TX);
		}
	}
}