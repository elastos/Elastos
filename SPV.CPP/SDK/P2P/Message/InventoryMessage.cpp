// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "InventoryMessage.h"
#include "GetDataMessage.h"
#include "GetBlocksMessage.h"
#include "PingMessage.h"

#include <P2P/Peer.h>
#include <Common/Log.h>
#include <Common/Utils.h>
#include <P2P/PeerManager.h>

#include <float.h>

#define MAX_BLOCKS_COUNT 500

namespace Elastos {
	namespace ElaWallet {

		InventoryMessage::InventoryMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool InventoryMessage::Accept(const bytes_t &msg) {
			ByteStream stream(msg);
			uint32_t type;

			uint32_t count;
			if (!stream.ReadUint32(count)) {
				_peer->error("inv msg read count fail");
				return false;
			}

			if (count > MAX_GETDATA_HASHES) {
				_peer->error("dropping inv message, {} is too many items, max is {}", count, MAX_GETDATA_HASHES);
				return false;
			}

			std::vector<uint256> transactions, blocks;
			uint256 hash;
			size_t i;

			for (i = 0; i < count; i++) {
				if (!stream.ReadUint32(type)) {
					_peer->error("inv msg read type fail");
					return false;
				}

				if (!stream.ReadBytes(hash)) {
					_peer->error("inv msg read hash fail, type = {}", type);
					return false;
				}

				if (type == inv_block) {
					blocks.push_back(hash);
				} else if (type == inv_tx) {
					transactions.push_back(hash);
				}
			}

			if (transactions.size() > 0 && !_peer->SentFilter() && !_peer->SentMempool() && !_peer->SentGetblocks()) {
				_peer->error("got inv message before loading a filter");
				return false;
			} else if (transactions.size() > 10000) { // sanity check
				_peer->error("too many transactions, disconnecting");
				return false;
			} else if (_peer->GetCurrentBlockHeight() > 0 && blocks.size() > 2 && blocks.size() < MAX_BLOCKS_COUNT &&
					   _peer->GetCurrentBlockHeight() + _peer->GetKnownBlockHashes().size() + blocks.size() <
					   _peer->GetLastBlock()) {
				_peer->error("non-standard inv, {} is fewer block hash(es) than expected", blocks.size());
				return false;
			} else {
				if (blocks.size() >= MAX_BLOCKS_COUNT && _peer->WaitingBlocks()) {
					_peer->error("got inv message before previous getdata respond");
					return false;
				}

				if (!_peer->SentFilter() && !_peer->SentGetblocks())
					blocks.clear();
				if (blocks.size() > 0) {
					if (blocks.size() == 1 && _peer->LastBlockHash() == blocks[0]) blocks.clear();
					if (blocks.size() == 1) _peer->SetLastBlockHash(blocks[0]);
				}

				for (i = 0; i < blocks.size(); i++) {
					// remember blockHashes in case we need to re-request them with an updated bloom filter
					_peer->AddKnownBlockHash(blocks[i]);
				}

				while (_peer->GetKnownBlockHashes().size() > MAX_GETDATA_HASHES) {
					_peer->KnownBlockHashesRemoveRange(0, _peer->GetKnownBlockHashes().size() / 3);
				}

				if (_peer->NeedsFilterUpdate()) blocks.clear();

				std::vector<uint256> txHashes;
				for (i = 0; i < transactions.size(); i++) {
					if (_peer->KnownTxHashSet().find(transactions[i]) != _peer->KnownTxHashSet().end()) {
						FireHasTx(transactions[i]);
					} else {
						txHashes.push_back(transactions[i]);
					}
				}

				_peer->info("got inv with {} tx {} block item(s)", txHashes.size(), blocks.size());
				_peer->AddKnownTxHashes(txHashes);
				if (txHashes.size() > 0 || blocks.size() > 0) {
					GetDataParameter getDataParam(txHashes, blocks);
					_peer->SendMessage(MSG_GETDATA, getDataParam);
				}

				// to improve chain download performance, if we received 500 block hashes, request the next 500 block hashes
				if (blocks.size() >= MAX_BLOCKS_COUNT) {
					GetBlocksParameter param;
					param.locators.push_back(blocks.back());
					param.locators.push_back(blocks.front());
					param.hashStop = 0;

					_peer->SendMessage(MSG_GETBLOCKS, param);
				}

				if (transactions.size() > 0 && !_peer->GetMemPoolCallback().empty()) {
					_peer->info("got initial mempool response");
					PingParameter pingParameter(_peer->GetPeerManager()->GetLastBlockHeight(),
												_peer->GetMemPoolCallback());
					_peer->SendMessage(MSG_PING, pingParameter);
					_peer->ResetMemPool();
				}
			}

			return true;
		}

		void InventoryMessage::Send(const SendMessageParameter &param) {
			const InventoryParameter &invParam = static_cast<const InventoryParameter &>(param);

			size_t txCount;
			size_t knownCount = _peer->KnownTxHashes().size();

			_peer->AddKnownTxHashes(invParam.txHashes);
			txCount = _peer->KnownTxHashes().size() - knownCount;

			if (txCount > 0) {
				ByteStream stream;

				stream.WriteUint32(uint32_t(txCount));
				for (size_t i = 0; i < txCount; i++) {
					stream.WriteUint32(uint32_t(inv_tx));  // version
					stream.WriteBytes(_peer->KnownTxHashes()[knownCount + i]);
				}

				_peer->info("sending inv tx count={} type={}", txCount, inv_tx);
				SendMessage(stream.GetBytes(), Type());
			}
		}

		std::string InventoryMessage::Type() const {
			return MSG_INV;
		}

	}
}