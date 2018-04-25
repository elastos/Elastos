//
// Created by jzh on 18-4-25.
//

#include "GetAddressMessage.h"



// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <float.h>

#include "BRPeer.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"
#include "BRArray.h"

#include "GetAddressMessage.h"

namespace Elastos {
	namespace SDK {

		GetAddressMessage::GetAddressMessage() {

		}

		int GetAddressMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			peer_log(peer, "GetAddressMessage.Accept");

			BRPeerContext *ctx = (BRPeerContext *)peer;

			peer_log(peer, "got getaddr");
			ctx->manager->peerMessages->BRPeerSendAddressMessage(peer);
			return 1;
		}

		void GetAddressMessage::Send(BRPeer *peer) {
			peer_log(peer, "GetAddressMessage.Send");

			((BRPeerContext *)peer)->sentGetaddr = 1;
			BRPeerSendMessage(peer, NULL, 0, MSG_GETADDR);
		}

	}
}