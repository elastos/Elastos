// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRPeerMessages.h"

#include "GetDataMessage.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {


		void GetDataMessage::SendGetData(BRPeer *peer, const UInt256 *txHashes,
										 size_t txCount, const UInt256 *blockHashes, size_t blockCount) {
			size_t msgLen = sizeof(uint8_t) + sizeof(UInt256);
			uint8_t msg[msgLen];

			size_t i, off;
			for (i = 0, off = 0; i < txCount; i++) {
				msg[off] = inv_tx;
				off += sizeof(uint8_t);
				UInt256Set(&msg[off], txHashes[i]);
				off += sizeof(UInt256);

				BRPeerSendMessage(peer, msg, off, MSG_GETDATA);
			}

			for (i = 0, off = 0; i < blockCount; i++) {
				msg[off] = inv_block;	//todo core get message use inv_filtered_block here, figure out differences later
				off += sizeof(uint8_t);
				UInt256Set(&msg[off], blockHashes[i]);
				off += sizeof(UInt256);

				BRPeerSendMessage(peer, msg, off, MSG_GETDATA);
			}

			((BRPeerContext *) peer)->sentGetdata = 1;
		}

		int GetDataMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			return 0;
		}

		void GetDataMessage::Send(BRPeer *peer) {

		}
	}
}