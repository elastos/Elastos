// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <BRPeerMessages.h>

#include "BRPeerMessages.h"

#include "PeerMessageManager.h"
#include "TransactionMessage.h"
#include "MerkleBlockMessage.h"
#include "VersionMessage.h"
#include "AddressMessage.h"
#include "InventoryMessage.h"
#include "AddressMessage.h"
#include "BloomFilterMessage.h"
#include "NotFoundMessage.h"
#include "GetBlocksMessage.h"
#include "GetDataMessage.h"
#include "PingMessage.h"
#include "PongMessage.h"

namespace Elastos {
	namespace SDK {

		namespace {
			int PeerAcceptTxMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());
				boost::function<int(BRPeer *, const uint8_t *, size_t)> fun =
						boost::bind(&TransactionMessage::Accept, message, _1, _2, _3);
				return fun(peer, msg, msgLen);
			}

			void PeerSendTxMessage(BRPeer *peer, BRTransaction *tx) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());
				boost::function<void(BRPeer *, void *)> fun =
						boost::bind(&TransactionMessage::Send, message, _1, _2);

				fun(peer, tx);
			}

			int PeerAcceptMerkleblockMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				MerkleBlockMessage *message = static_cast<MerkleBlockMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_MERKLEBLOCK).get());
				boost::function<void(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&MerkleBlockMessage::Accept, message, _1, _2, _3);

				fun(peer, msg, msgLen);
			}

			int PeerAcceptVersionMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				VersionMessage *message = static_cast<VersionMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERSION).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&VersionMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void PeerSendVersionMessage(BRPeer *peer) {
				VersionMessage *message = static_cast<VersionMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERSION).get());
				boost::function<void(BRPeer *peer)> fun =
						boost::bind(&VersionMessage::Send, message, _1);

				fun(peer);
			}

			int PeerAcceptAddressMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				AddressMessage *message = static_cast<AddressMessage *>(
						PeerMessageManager::instance().getMessage(MSG_ADDR).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&AddressMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void PeerSendAddressMessage(BRPeer *peer) {
				AddressMessage *message = static_cast<AddressMessage *>(
						PeerMessageManager::instance().getMessage(MSG_ADDR).get());
				boost::function<void(BRPeer *peer)> fun =
						boost::bind(&AddressMessage::Send, message, _1);

				fun(peer);
			}

			int PeerAcceptInventoryMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				InventoryMessage *message = static_cast<InventoryMessage *>(
						PeerMessageManager::instance().getMessage(MSG_INV).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&InventoryMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void PeerSendInventoryMessage(BRPeer *peer, const UInt256 *txHashes, size_t txCount) {
				InventoryMessage *message = static_cast<InventoryMessage *>(
						PeerMessageManager::instance().getMessage(MSG_INV).get());
				boost::function<void(BRPeer *peer, const UInt256 txHashes[], size_t txCount)> fun =
						boost::bind(&InventoryMessage::Send, message, _1, _2, _3);

				fun(peer, txHashes, txCount);
			}

			int PeerAcceptNotFoundMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				NotFoundMessage *message = static_cast<NotFoundMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETADDR).get());
				boost::function<int(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&NotFoundMessage::Accept, message, _1, _2, _3);

				return fun(peer, msg, msgLen);
			}

			void PeerSendFilterload(BRPeer *peer, BRBloomFilter *filter) {
				BloomFilterMessage *message = static_cast<BloomFilterMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_FILTERLOAD).get());
				boost::function<void(BRPeer *, void *)> fun =
						boost::bind(&BloomFilterMessage::Send, message, _1, _2);

				fun(peer, filter);
			}

			void PeerSendGetblocks(BRPeer *peer, const UInt256 *locators, size_t locatorsCount, UInt256 hashStop) {
				GetBlocksMessage *message = static_cast<GetBlocksMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETBLOCKS).get());
				boost::function<void(BRPeer *, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop)> fun =
						boost::bind(&GetBlocksMessage::SendGetBlocks, message, _1, _2, _3, _4);

				fun(peer, locators, locatorsCount, hashStop);
			}

			void PeerSendGetdata(BRPeer *peer, const UInt256 *txHashes, size_t txCount,
								 const UInt256 *blockHashes, size_t blockCount) {
				GetDataMessage *message = static_cast<GetDataMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETDATA).get());
				boost::function<void(BRPeer *peer, const UInt256 txHashes[], size_t txCount,
									 const UInt256 blockHashes[], size_t blockCount)> fun =
						boost::bind(&GetDataMessage::SendGetData, message, _1, _2, _3, _4, _5);

				fun(peer, txHashes, txCount, blockHashes, blockCount);
			}

			void PeerSendPingMessage(BRPeer *peer, void *info, void (*pongCallback)(void *info, int success)) {
				PingMessage *message = static_cast<PingMessage *>(
						PeerMessageManager::instance().getMessage(MSG_PING).get());
				boost::function<void(BRPeer *peer, void *info, void (*pongCallback)(void *info, int success))> fun =
						boost::bind(&PingMessage::sendPing, message, _1, _2, _3);

				fun(peer, info, pongCallback);
			}

			int PeerAcceptPingMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				PingMessage *message = static_cast<PingMessage *>(
						PeerMessageManager::instance().getMessage(MSG_PING).get());
				boost::function<void(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&PingMessage::Accept, message, _1, _2, _3);

				fun(peer, msg, msgLen);
			}

			int PeerAcceptPongMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				PongMessage *message = static_cast<PongMessage *>(
						PeerMessageManager::instance().getMessage(MSG_PONG).get());
				boost::function<void(BRPeer *peer, const uint8_t *msg, size_t msgLen)> fun =
						boost::bind(&PongMessage::Accept, message, _1, _2, _3);

				fun(peer, msg, msgLen);
			}
		}

		PeerMessageManager PeerMessageManager::_instance = PeerMessageManager();

		PeerMessageManager::PeerMessageManager() {
		}

		PeerMessageManager::~PeerMessageManager() {
		}

		void PeerMessageManager::initMessages(BRPeerManager *peerManager) {

			peerManager->peerMessages = BRPeerMessageNew();

			peerManager->peerMessages->BRPeerAcceptTxMessage = PeerAcceptTxMessage;
			peerManager->peerMessages->BRPeerSendTxMessage = PeerSendTxMessage;
			_wrapperMessages[MSG_TX] = WrapperMessagePtr(new TransactionMessage);

			peerManager->peerMessages->BRPeerAcceptMerkleblockMessage = PeerAcceptMerkleblockMessage;
			_wrapperMessages[MSG_MERKLEBLOCK] = WrapperMessagePtr(new MerkleBlockMessage);

			peerManager->peerMessages->BRPeerAcceptVersionMessage = PeerAcceptVersionMessage;
			peerManager->peerMessages->BRPeerSendVersionMessage = PeerSendVersionMessage;
			_messages[MSG_VERSION] = MessagePtr(new VersionMessage);

			peerManager->peerMessages->BRPeerAcceptInventoryMessage = PeerAcceptInventoryMessage;
			peerManager->peerMessages->BRPeerSendInventoryMessage = PeerSendInventoryMessage;
			_messages[MSG_INV] = MessagePtr(new InventoryMessage);

			peerManager->peerMessages->BRPeerAcceptAddressMessage = PeerAcceptAddressMessage;
			peerManager->peerMessages->BRPeerSendAddressMessage = PeerSendAddressMessage;
			_messages[MSG_ADDR] = MessagePtr(new AddressMessage);

			peerManager->peerMessages->BRPeerAcceptNotFoundMessage = PeerAcceptNotFoundMessage;
			_messages[MSG_NOTFOUND] = MessagePtr(new NotFoundMessage);

			peerManager->peerMessages->BRPeerSendFilterloadMessage = PeerSendFilterload;
			_wrapperMessages[MSG_FILTERLOAD] = WrapperMessagePtr(new BloomFilterMessage);

			peerManager->peerMessages->BRPeerSendGetblocksMessage = PeerSendGetblocks;
			_messages[MSG_GETBLOCKS] = MessagePtr(new GetBlocksMessage);

			//use same message with getblocks
			peerManager->peerMessages->BRPeerSendGetheadersMessage = PeerSendGetblocks;

			peerManager->peerMessages->BRPeerSendGetdataMessage = PeerSendGetdata;
			_messages[MSG_GETDATA] = MessagePtr(new GetDataMessage);

			peerManager->peerMessages->BRPeerSendPingMessage = PeerSendPingMessage;
			peerManager->peerMessages->BRPeerAcceptPingMessage = PeerAcceptPingMessage;
			_messages[MSG_PING] = MessagePtr(new PingMessage);

			peerManager->peerMessages->BRPeerAcceptPongMessage = PeerAcceptPongMessage;
			_messages[MSG_PONG] = MessagePtr(new PongMessage);
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