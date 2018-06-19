// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetBlocksMessage.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		int GetBlocksMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			return 0;
		}

		void GetBlocksMessage::Send(BRPeer *peer) {

		}

		void GetBlocksMessage::SendGetBlocks(BRPeer *peer, const UInt256 *locators,
											 size_t locatorsCount, UInt256 hashStop) {
			size_t i, off = 0;
			size_t msgLen = sizeof(uint32_t) + sizeof(*locators) * locatorsCount + sizeof(hashStop);
			uint8_t msg[msgLen];

			UInt32SetLE(&msg[off], uint32_t(locatorsCount));
			off += sizeof(uint32_t);

			for (i = 0; i < locatorsCount; i++) {
				UInt256Set(&msg[off], locators[i]);
				off += sizeof(UInt256);
			}

			UInt256Set(&msg[off], hashStop);
			off += sizeof(UInt256);

			if (locatorsCount > 0) {
				Log::getLogger()->warn("calling getblocks with {} locators: [{},{} {}]",
									   locatorsCount,
									   Utils::UInt256ToString(locators[0]),
									   (locatorsCount > 2 ? " ...," : ""),
									   (locatorsCount > 1 ? Utils::UInt256ToString(locators[locatorsCount - 1]) : ""));
				BRPeerSendMessage(peer, msg, off, MSG_GETBLOCKS);
			}
		}
	}
}