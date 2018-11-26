// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "HeadersMessage.h"
#include "GetBlocksMessage.h"
#include "GetHeadersMessage.h"

#include <SDK/Plugin/Registry.h>
#include <SDK/Common/Utils.h>
#include <SDK/P2P/Peer.h>
#include <SDK/P2P/PeerManager.h>
#include <Core/BRAddress.h>

#define BLOCK_MAX_TIME_DRIFT      (2*60*60) // the furthest in the future a block is allowed to be timestamped

namespace Elastos {
	namespace ElaWallet {

		HeadersMessage::HeadersMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool HeadersMessage::Accept(const CMBlock &msg) {
			size_t off = 0, count = (size_t)BRVarInt(msg, msg.GetSize(), &off);
			int r = 1;

			if (off == 0 || off + 81*count > msg.GetSize()) {
				_peer->Perror("malformed headers message, length is %zu, should be %zu for %zu header(s)", msg.GetSize(),
						 BRVarIntSize(count) + 81*count, count);
				r = 0;
			} else {
				_peer->Pinfo("got %zu header(s)", count);

				// To improve chain download performance, if this message contains 2000 headers then request the next 2000
				// headers immediately, and switch to requesting blocks when we receive a header newer than earliestKeyTime
				uint32_t timestamp = (count > 0) ? UInt32GetLE(&msg[off + 81*(count - 1) + 68]) : 0;

				if (count >= 2000 || (timestamp > 0 && timestamp + 7*24*60*60 + BLOCK_MAX_TIME_DRIFT >= _peer->GetEarliestKeyTime())) {
					size_t last = 0;
					time_t now = time(NULL);
					UInt256 locators[2];

					BRSHA256_2(&locators[0], &msg[off + 81*(count - 1)], 80);
					BRSHA256_2(&locators[1], &msg[off], 80);

					if (timestamp > 0 && timestamp + 7*24*60*60 + BLOCK_MAX_TIME_DRIFT >= _peer->GetEarliestKeyTime()) {
						// request blocks for the remainder of the chain
						timestamp = (++last < count) ? UInt32GetLE(&msg[off + 81*last + 68]) : 0;

						while (timestamp > 0 && timestamp + 7*24*60*60 + BLOCK_MAX_TIME_DRIFT < _peer->GetEarliestKeyTime()) {
							timestamp = (++last < count) ? UInt32GetLE(&msg[off + 81*last + 68]) : 0;
						}

						BRSHA256_2(&locators[0], &msg[off + 81*(last - 1)], 80);
						GetBlocksParameter param;
						param.locators.push_back(locators[0]);
						param.locators.push_back(locators[1]);
						param.hashStop = UINT256_ZERO;
						_peer->SendMessage(MSG_GETBLOCKS, param);
					} else {
						GetHeadersParameter param;
						param.locators.push_back(locators[0]);
						param.locators.push_back(locators[1]);
						param.hashStop = UINT256_ZERO;
						_peer->SendMessage(MSG_GETHEADERS, param);
					}

					for (size_t i = 0; r && i < count; i++) {
						PeerManager *manager = _peer->getPeerManager();
						MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(manager->GetPluginType()));
						ByteStream stream(&msg[off + 81 + i], 81, false);

						if (!block->Deserialize(stream)) {
							_peer->Perror("merkle block deserialize");
							return false;
						}

						if (! block->isValid((uint32_t)now)) {
							_peer->Perror("Invalid block header: {}", Utils::UInt256ToString(block->getHash()));
							return false;
						}
						FireRelayedBlock(block);
					}
				} else {
					_peer->Perror("non-standard headers message, %zu is fewer header(s) than expected", count);
					return false;
				}
			}

			return true;
		}

		void HeadersMessage::Send(const SendMessageParameter &param) {
		}

		std::string HeadersMessage::Type() const {
			return MSG_HEADERS;
		}
	}
}
