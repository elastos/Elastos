// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "BRPeerMessages.h"

#include "PeerMessageManager.h"
#include "VerackMessage.h"

namespace Elastos {
	namespace SDK {

		namespace {
			int BRPeerAcceptVerackMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				VerackMessage *message = static_cast<VerackMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERACK).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&VerackMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}
		}

		PeerMessageManager PeerMessageManager::_instance = PeerMessageManager();

		PeerMessageManager::PeerMessageManager() {
		}

		PeerMessageManager::~PeerMessageManager() {
		}

		void PeerMessageManager::initMessages() {

			GlobalMessages->BRPeerAcceptVerackMessage = BRPeerAcceptVerackMessage;
			_messages[MSG_VERACK] = MessagePtr(new VerackMessage);;
		}

		PeerMessageManager &PeerMessageManager::instance() {
			return _instance;
		}

		const PeerMessageManager::MessagePtr &PeerMessageManager::getMessage(const std::string &message) {
			return _messages[message];
		}
	}
}