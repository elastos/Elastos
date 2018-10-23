// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetHeadersMessage.h"

namespace Elastos {
	namespace ElaWallet {

		GetHeadersMessage::GetHeadersMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetHeadersMessage::Accept(const CMBlock &msg) {
			//fixme [refactor]
			return false;
		}

		void GetHeadersMessage::Send(const SendMessageParameter &param) {
			//fixme [refactor]
		}

		std::string GetHeadersMessage::Type() const {
			return MSG_GETHEADERS;
		}
	}
}
