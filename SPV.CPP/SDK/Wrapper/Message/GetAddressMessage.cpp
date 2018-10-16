//
// Created by 延明智 on 2018/10/16.
//
// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "GetAddressMessage.h"

namespace Elastos {
	namespace ElaWallet {

		GetAddressMessage::GetAddressMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool GetAddressMessage::Accept(const CMBlock &msg) {
			//fixme [refactor]
			return false;
		}

		void GetAddressMessage::Send(const SendMessageParameter &param) {
			//fixme [refactor]
		}

		std::string GetAddressMessage::Type() const {
			return MSG_GETADDR;
		}
	}
}
