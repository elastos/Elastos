// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "FilterLoadMessage.h"

#include <SDK/P2P/Peer.h>
#include <SDK/Base/BloomFilter.h>
#include <SDK/Common/ByteStream.h>

namespace Elastos {
	namespace ElaWallet {

		FilterLoadMessage::FilterLoadMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool FilterLoadMessage::Accept(const CMBlock &msg) {
			_peer->error("dropping {} message", Type());
			return false;
		}

		void FilterLoadMessage::Send(const SendMessageParameter &param) {
			const FilterLoadParameter &filterLoadParameter = static_cast<const FilterLoadParameter &>(param);
			ByteStream stream;
			filterLoadParameter.Filter->Serialize(stream);
			_peer->SetSentFilter(true);
			_peer->SetSentMempool(false);
			SendMessage(stream.GetBuffer(), Type());
		}

		std::string FilterLoadMessage::Type() const {
			return MSG_FILTERLOAD;
		}
	}
}