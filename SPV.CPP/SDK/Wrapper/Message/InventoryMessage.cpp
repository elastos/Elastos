// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <float.h>
#include <BRPeerMessages.h>
#include <BRPeerManager.h>

#include "Peer.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"
#include "BRArray.h"

#include "InventoryMessage.h"
#include "Log.h"
#include "Utils.h"

#define MAX_BLOCKS_COUNT 100  //note max blocks count is 500 in btc while 100 in ela

namespace Elastos {
	namespace ElaWallet {

		InventoryMessage::InventoryMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool InventoryMessage::Accept(const CMBlock &msg) {
			size_t off = 0;
			inv_type type;

			size_t count = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);

			if (off + count * 36 > msg.GetSize()) {
				_peer->Perror("malformed inv message, length is {}, should be {} for {} item(s)", msg.GetSize(),
					off + count * 36, count);
				return false;
			} else if (count > MAX_GETDATA_HASHES) {
				_peer->Perror("dropping inv message, {} is too many items, max is {}", count,
									   MAX_GETDATA_HASHES);
				return false;
			} else {
				const uint8_t *transactions[count], *blocks[count];
				size_t i, txCount = 0, blockCount = 0;

				_peer->Pinfo("got inv with {} item(s)", count);

				for (i = 0; i < count; i++) {
					type = inv_type(UInt32GetLE(&msg[off]));
					off += sizeof(uint32_t);

					if (type == inv_block) {
						blocks[blockCount++] = &msg[off];
						off += sizeof(UInt256);
					} else if (type == inv_tx) {
						transactions[txCount++] = &msg[off];
						off += sizeof(UInt256);
					}
				}

				if (txCount > 0 && !_peer->SentFilter() && !_peer->SentMempool() && !_peer->SentGetblocks()) {
					_peer->Perror("got inv message before loading a filter");
					return false;
				} else if (txCount > 10000) { // sanity check
					_peer->Perror("too many transactions, disconnecting");
					return false;
				} else if (_peer->GetCurrentBlockHeight() > 0 && blockCount > 2 && blockCount < MAX_BLOCKS_COUNT &&
						   _peer->GetCurrentBlockHeight() + _peer->GetKnownBlockHashes().size() + blockCount < _peer->getLastBlock()) {
					_peer->Perror("non-standard inv, {} is fewer block hash(es) than expected", blockCount);
					return false;
				} else {
					if (!_peer->SentFilter() && !_peer->SentGetblocks()) blockCount = 0;
					UInt256 blockHash;
					if (blockCount > 0) {
						UInt256Get(&blockHash, blocks[0]);
						if (blockCount == 1 && UInt256Eq(&(_peer->GetLastBlockHash()), &blockHash)) blockCount = 0;
						if (blockCount == 1) _peer->SetLastBlockHash(*(const UInt256 *)blocks[0]);
					}

					UInt256 hash;
					std::vector<UInt256> blockHashes, txHashes;

					blockHashes.reserve(blockCount);
					txHashes.reserve(txCount);

					for (i = 0; i < blockCount; i++) {
						blockHashes.push_back(*(UInt256 *)blocks[i]);
						// remember blockHashes in case we need to re-request them with an updated bloom filter
						_peer->AddKnownBlockHash(*(const UInt256 *)blocks[i]);
					}

					while (_peer->GetKnownBlockHashes().size() > MAX_GETDATA_HASHES) {
						_peer->KnownBlockHashesRemoveRange(0, _peer->GetKnownBlockHashes().size() / 3);
					}

					if (_peer->NeedsFilterUpdate()) blockCount = 0;

					for (i = 0; i < txCount; i++) {
						hash = *(const UInt256 *)transactions[i];

						// fixme [refactor]
//						if (_peer->GetKnownTxHashSet().Contains(hash)) {
//							FireHasTx(hash);
//						} else {
//							txHashes.push_back(hash);
//						}
					}

					_peer->Pinfo("got inv with txCount={}, blockCount={}", txHashes.size(), blockCount);
					// fixme [refactor]
//					BRPeerAddKnownTxHashes(peer, txHashes, txHashes.size());
//					if (txHashes.size() > 0 || blockCount > 0)
//						ctx->manager->peerMessages->BRPeerSendGetdataMessage(peer, txHashes, j, blockHashes, blockCount);

					// to improve chain download performance, if we received 500 block hashes, request the next 500 block hashes
					if (blockCount >= MAX_BLOCKS_COUNT) {
						UInt256 locators[] = {blockHashes[blockCount - 1], blockHashes[0]};

						// fixme [refactor]
//						ctx->manager->peerMessages->BRPeerSendGetblocksMessage(peer, locators, 2, UINT256_ZERO);
					}

					// fixme [refactor]
//					if (txCount > 0 && _peer->mempoolCallback) {
//						_peer->Pinfo("got initial mempool response");
//						ctx->manager->peerMessages->BRPeerSendPingMessage(peer, ctx->mempoolInfo, ctx->mempoolCallback);
//						ctx->mempoolCallback = nullptr;
//						ctx->mempoolTime = DBL_MAX;
//					}
				}
			}

			return true;
		}

		void InventoryMessage::Send(const SendMessageParameter &param) {
			const InventoryParameter &invParam = static_cast<const InventoryParameter &>(param);

			size_t txCount = invParam.txHashes.size();
			size_t knownCount = _peer->GetKnownTxHashes().size();

			// fixme [refactor]
//			BRPeerAddKnownTxHashes(peer, txHashes, txCount);
			txCount = _peer->GetKnownTxHashes().size() - knownCount;

			if (txCount > 0) {
				size_t i;
				ByteStream stream;

				stream.writeUint32(uint32_t(txCount));
				for (size_t i = 0; i < txCount; i++) {
					stream.writeUint32(uint32_t(inv_tx));  // version
					stream.writeBytes(&_peer->GetKnownTxHashes()[knownCount + i], sizeof(UInt256));
				}

				_peer->Pinfo("sending inv tx count={} type={}", txCount, inv_tx);
				SendMessage(stream.getBuffer(), Type());
			}
		}

		std::string InventoryMessage::Type() const {
			return MSG_INV;
		}

	}
}