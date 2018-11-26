// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetAddressMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/P2P/PeerManager.h>

namespace Elastos {
	namespace ElaWallet {

		GetAddressMessage::GetAddressMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetAddressMessage::Accept(const CMBlock &msg) {
			_peer->Pinfo("got getaddr");
			_peer->SendMessage(MSG_ADDR, Message::DefaultParam);
			return true;
		}

		void GetAddressMessage::Send(const SendMessageParameter &param) {
			_peer->SetSentGetaddr(true);
			_peer->getPeerManager()->SetKeepAliveTimestamp(time(nullptr));
			SendMessage(CMBlock(), Type());
		}

		std::string GetAddressMessage::Type() const {
			return MSG_GETADDR;
		}
	}
}
