// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arpa/inet.h>
#include <Core/BRPeer.h>
#include "BRPeer.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"
#include "BRArray.h"

#include "AddressMessage.h"

namespace Elastos {
	namespace ElaWallet {
		AddressMessage::AddressMessage() {

		}

		int AddressMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *)peer;

			size_t off = 0;
			size_t count = UInt64GetLE(&msg[off]);
			off += sizeof(uint64_t);

			int r = 1;
			if (off == 0 || off + count*30 > msgLen) {
				peer_log(peer, "malformed addr message, length is %zu, should be %zu for %zu address(es)", msgLen,
						 BRVarIntSize(count) + 30*count, count);
				r = 0;
			}
			else if (count > 1000) {
				peer_log(peer, "dropping addr message, %zu is too many addresses, max is 1000", count);
			}
			else if (ctx->sentGetaddr) { // simple anti-tarpitting tactic, don't accept unsolicited addresses
				BRPeer peers[count], p;
				size_t peersCount = 0;
				time_t now = time(NULL);

				peer_log(peer, "got addr with %zu address(es)", count);

				for (size_t i = 0; i < count; i++) {
					p.timestamp = UInt64GetLE(&msg[off]);
					off += sizeof(uint64_t);
					p.services = UInt64GetLE(&msg[off]);
					off += sizeof(uint64_t);
					UInt128Get(&p.address, &msg[off]);
					off += sizeof(UInt128);
					p.port = UInt16GetLE(&msg[off]);
					off += sizeof(uint16_t);

					char host[INET6_ADDRSTRLEN] = {0};
					if ((p.address.u64[0] == 0 && p.address.u16[4] == 0 && p.address.u16[5] == 0xffff))
						inet_ntop(AF_INET, &p.address.u32[3], host, sizeof(host));
					else
						inet_ntop(AF_INET6, &p.address, host, sizeof(host));

					peer_dbg(peer, "peers[%zu] = %s, timestamp = %zu, port = %d, services = %zu",
							 i, host, p.timestamp, p.port, p.services);

					if (! (p.services & SERVICES_NODE_NETWORK)) continue; // skip peers that don't carry full blocks
					if (! (p.address.u64[0] == 0 && p.address.u16[4] == 0 && p.address.u16[5] == 0xffff))
						continue; // ignore IPv6 for now
					if (p.address.u64[0] == 0 && p.address.u16[4] == 0 && p.address.u16[5] == 0xffff &&
						p.address.u8[12] == 127 && p.address.u8[13] == 0 &&
						p.address.u8[14] == 0 && p.address.u8[15] == 1) {
						peer_log(peer, "drop peers[%zu]", i);
						continue;
					}

					// if address time is more than 10 min in the future or unknown, set to 5 days old
					if (p.timestamp > now + 10*60 || p.timestamp == 0) p.timestamp = now - 5*24*60*60;
					p.timestamp -= 2*60*60; // subtract two hours
					peers[peersCount++] = p; // add it to the list
				}

				if (peersCount > 0 && ctx->relayedPeers) ctx->relayedPeers(ctx->info, peers, peersCount);
			}

			return r;
		}

		void AddressMessage::Send(BRPeer *peer) {
			uint8_t msg[BRVarIntSize(0)];
			size_t msgLen = BRVarIntSet(msg, sizeof(msg), 0);

			//TODO: send peer addresses we know about
//			BRPeerSendMessage(peer, msg, msgLen, MSG_ADDR);
		}


	}
}
