// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <BRPeerMessages.h>

#include "BRPeerMessages.h"

#include "PeerMessageManager.h"
#include "VerackMessage.h"
#include "TransactionMessage.h"
#include "MerkleBlockMessage.h"

namespace Elastos {
	namespace SDK {

		namespace {
			int BRPeerAcceptVerackMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				VerackMessage *message = static_cast<VerackMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERACK).get());
				boost::function<int(BRPeer *, const uint8_t *, size_t)> fun =
						boost::bind(&VerackMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void BRPeerSendVerackMessage(BRPeer *peer) {
				VerackMessage *message = static_cast<VerackMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERACK).get());
				boost::function<void(BRPeer *)> fun =
						boost::bind(&VerackMessage::Send, message, _1);

				fun(peer);
			}

			int BRPeerAcceptTxMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());
				boost::function<int(BRPeer *, const uint8_t *, size_t)> fun =
						boost::bind(&TransactionMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void BRPeerSendTxMessage(BRPeer *peer, BRTransaction *tx) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());
				boost::function<void(BRPeer *, void *)> fun =
						boost::bind(&TransactionMessage::Send, message, _1, _2);

				fun(peer, tx);
			}

			int BRPeerAcceptMerkleblockMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				MerkleBlockMessage *message = static_cast<MerkleBlockMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_MERKLEBLOCK).get());
				boost::function<void(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&MerkleBlockMessage::Accept, message, _1, _2, _3);

				fun(peer, msg, msgLen);
			}

		}

		PeerMessageManager PeerMessageManager::_instance = PeerMessageManager();

		PeerMessageManager::PeerMessageManager() {
		}

		PeerMessageManager::~PeerMessageManager() {
		}

		void PeerMessageManager::initMessages(BRPeerManager *peerManager) {

			peerManager->peerMessages->BRPeerAcceptVerackMessage = BRPeerAcceptVerackMessage;
			peerManager->peerMessages->BRPeerSendVerackMessage = BRPeerSendVerackMessage;
			_messages[MSG_VERACK] = MessagePtr(new VerackMessage);

			peerManager->peerMessages->BRPeerAcceptTxMessage = BRPeerAcceptTxMessage;
			peerManager->peerMessages->BRPeerSendTxMessage = BRPeerSendTxMessage;
			_wrapperMessages[MSG_TX] = WrapperMessagePtr(new TransactionMessage);

			peerManager->peerMessages->BRPeerAcceptMerkleblockMessage = BRPeerAcceptMerkleblockMessage;
			_wrapperMessages[MSG_MERKLEBLOCK] = WrapperMessagePtr(new MerkleBlockMessage);
		}

		PeerMessageManager &PeerMessageManager::instance() {
			return _instance;
		}

		const PeerMessageManager::WrapperMessagePtr &PeerMessageManager::getWrapperMessage(
				const std::string &message) {
			return _wrapperMessages[message];
		}

		const PeerMessageManager::MessagePtr &PeerMessageManager::getMessage(const std::string &message) {
			return _messages[message];
		}
	}
}