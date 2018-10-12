// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Peer.h"
#include "BloomFilter.h"
#include "BloomFilterMessage.h"
#include "ByteStream.h"

namespace Elastos {
	namespace ElaWallet {

		BloomFilterMessage::BloomFilterMessage(const MessagePeerPtr &peer) :
			Message(peer) {

		}

		bool BloomFilterMessage::Accept(const CMBlock &msg) {
			return false;
		}

		void BloomFilterMessage::Send(const SendMessageParameter &param) {
			const BloomFilterParameter &bloomFilterParameter = static_cast<const BloomFilterParameter &>(param);
			ByteStream byteStream;
			bloomFilterParameter.Filter->Serialize(byteStream);
			_peer->SetSentFilter(true);
			_peer->SetSentMempool(false);
			SendMessage(byteStream.getBuffer(), Type());
		}

		std::string BloomFilterMessage::Type() const {
			return MSG_FILTERLOAD;
		}
	}
}