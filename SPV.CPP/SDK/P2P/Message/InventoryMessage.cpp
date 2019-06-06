// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "InventoryMessage.h"
#include "GetDataMessage.h"
#include "GetBlocksMessage.h"
#include "PingMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/P2P/PeerManager.h>

#include <Core/BRArray.h>
#include <float.h>

#define MAX_BLOCKS_COUNT 100  //note max blocks count is 500 in btc while 100 in ela

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

				if (type == inv_block || type == inv_confirmd_block) {
					if (!stream.ReadBytes(hash)) {
						_peer->error("inv msg read block hash fail");
						return false;
					}
					blocks.push_back(hash);
				} else if (type == inv_tx) {
					if (!stream.ReadBytes(hash)) {
						_peer->error("inv msg read tx hash fail");
						return false;
					}
					transactions.push_back(hash);
				} else if (type == inv_address) {
					if (!stream.ReadBytes(hash)) {
						_peer->error("inv msg read address hash fail");
						return false;
					}
				} else {
					if (!stream.ReadBytes(hash)) {
						_peer->error("inv msg read other '{}' hash fail", type);
						return false;
					}
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
						FireHasTx(hash);
					} else {
						txHashes.push_back(hash);
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
					PingParameter pingParameter;
					pingParameter.callback = _peer->GetMemPoolCallback();
					pingParameter.lastBlockHeight = _peer->GetPeerManager()->GetLastBlockHeight();
					_peer->SendMessage(MSG_PING, pingParameter);
					_peer->ResetMemPool();
				}
			}

			return true;
		}

		void InventoryMessage::Send(const SendMessageParameter &param) {
			const InventoryParameter &invParam = static_cast<const InventoryParameter &>(param);

			size_t txCount = invParam.txHashes.size();
			size_t knownCount = _peer->KnownTxHashes().size();

			_peer->AddKnownTxHashes(invParam.txHashes);
			txCount = _peer->KnownTxHashes().size() - knownCount;

			if (txCount > 0) {
				size_t i;
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