// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include <cfloat>

#include "BRPeer.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"

#include "VersionMessage.h"

namespace Elastos {
	namespace ElaWallet {

		VersionMessage::VersionMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		int VersionMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			peer_log(peer, "VersionMessage.Accept");
			BRPeerContext *ctx = (BRPeerContext *)peer;
			size_t off = 0;
			uint16_t fromPort = 0;

			ctx->version = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);
			peer->services = UInt64GetLE(&msg[off]);
			off += sizeof(uint64_t);
			peer->timestamp = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);
			fromPort = UInt16GetLE(&msg[off]);
			off += sizeof(uint16_t);
			ctx->nonce = UInt64GetLE(&msg[off]);
			off += sizeof(uint64_t);
			ctx->lastblock = UInt64GetLE(&msg[off]);
			off += sizeof(uint64_t);
			peer_log(peer, "got version %" PRIu32 ", services %" PRIx64 "", ctx->version, peer->services);

			BRPeerSendVerackMessage(peer);
			return 1;
		}

		void VersionMessage::Send(BRPeer *peer) {
			peer_dbg(peer, "%s", "VersionMessage.Send");
			BRPeerContext *ctx = (BRPeerContext *)peer;
			size_t off = 0;
			uint8_t msg[35];

			UInt32SetLE(&msg[off], PROTOCOL_VERSION); // version
			off += sizeof(uint32_t);
			UInt64SetLE(&msg[off], ENABLED_SERVICES); // services
			off += sizeof(uint64_t);
			UInt32SetLE(&msg[off], time(NULL)); // timestamp
			off += sizeof(uint32_t);
			UInt16SetLE(&msg[off], peer->port); // port of remote peer
			off += sizeof(uint16_t);
			ctx->nonce = ((uint64_t)BRRand(0) << 32) | (uint64_t)BRRand(0); // random nonce
			UInt64SetLE(&msg[off], ctx->nonce);
			off += sizeof(uint64_t);
			UInt64SetLE(&msg[off], 0); // last block received ---spv start height
			off += sizeof(uint64_t);
			msg[off++] = 0; // relay transactions (0 for SPV bloom filter mode)
			peer_log(peer, "%d", (int)peer->port);

			BRPeerSendMessage(peer, msg, sizeof(msg), MSG_VERSION);
		}

	}
}