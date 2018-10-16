// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "VerackMessage.h"

namespace Elastos {
	namespace ElaWallet {

		VerackMessage::VerackMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool VerackMessage::Accept(const CMBlock &msg) {
			//fixme [refactor]
			return false;
		}

		void VerackMessage::Send(const SendMessageParameter &param) {
//fixme [refactor]
		}

		std::string VerackMessage::Type() const {
			return MSG_VERACK;
		}
	}
}
