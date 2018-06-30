// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <BRPeerMessages.h>
#include <SDK/ELACoreExt/ELAPeerManager.h>
#include <SDK/Plugin/Block/IdMerkleBlock.h>

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
#include "ELAMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		namespace {
			BRMerkleBlock *BRMerkleBlockNewWrapper() {
				ELAMerkleBlock *elablock = ELAMerkleBlockNew();
				return (BRMerkleBlock *)elablock;
			}

			void BRMerkleBlockFreeWrapper(BRMerkleBlock *block) {
				ELAMerkleBlock *elablock = (ELAMerkleBlock *)block;
				ELAMerkleBlockFree(elablock);
			}

			void setApplyFreeBlock(void *info, void *block) {
				ELAPeerManager *manager = (ELAPeerManager *) info;
				if (manager->Plugins.BlockType == "ELA") {
					ELAMerkleBlock *elablock = (ELAMerkleBlock *) block;
					ELAMerkleBlockFree(elablock);
				} else if(manager->Plugins.BlockType == "SideStandard") {
					IdMerkleBlock *idMerkleBlock = (IdMerkleBlock *) block;
					IdMerkleBlockFree(idMerkleBlock);
				}
			}

			int PeerAcceptTxMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());

				return message->Accept(peer, msg, msgLen);
			}

			void PeerSendTxMessage(BRPeer *peer, BRTransaction *tx) {
				TransactionMessage *message = static_cast<TransactionMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_TX).get());

				return message->Send(peer, tx);
			}

			int PeerAcceptMerkleblockMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				MerkleBlockMessage *message = static_cast<MerkleBlockMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_MERKLEBLOCK).get());

				return message->Accept(peer, msg, msgLen);
			}

			int PeerAcceptVersionMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				VersionMessage *message = static_cast<VersionMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERSION).get());

				return message->Accept(peer, msg, msgLen);
			}

			void PeerSendVersionMessage(BRPeer *peer) {
				VersionMessage *message = static_cast<VersionMessage *>(
						PeerMessageManager::instance().getMessage(MSG_VERSION).get());

				message->Send(peer);
			}

			int PeerAcceptAddressMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				AddressMessage *message = static_cast<AddressMessage *>(
						PeerMessageManager::instance().getMessage(MSG_ADDR).get());

				return message->Accept(peer, msg, msgLen);
			}

			void PeerSendAddressMessage(BRPeer *peer) {
				AddressMessage *message = static_cast<AddressMessage *>(
						PeerMessageManager::instance().getMessage(MSG_ADDR).get());

				message->Send(peer);
			}

			int PeerAcceptInventoryMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				InventoryMessage *message = static_cast<InventoryMessage *>(
						PeerMessageManager::instance().getMessage(MSG_INV).get());

				return message->Accept(peer, msg, msgLen);
			}

			void PeerSendInventoryMessage(BRPeer *peer, const UInt256 *txHashes, size_t txCount) {
				InventoryMessage *message = static_cast<InventoryMessage *>(
						PeerMessageManager::instance().getMessage(MSG_INV).get());

				message->Send(peer, txHashes, txCount);
			}

			int PeerAcceptNotFoundMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				NotFoundMessage *message = static_cast<NotFoundMessage *>(
						PeerMessageManager::instance().getMessage(MSG_NOTFOUND).get());

				return message->Accept(peer, msg, msgLen);
			}

			void PeerSendFilterload(BRPeer *peer, BRBloomFilter *filter) {
				BloomFilterMessage *message = static_cast<BloomFilterMessage *>(
						PeerMessageManager::instance().getWrapperMessage(MSG_FILTERLOAD).get());

				message->Send(peer, filter);
			}

			void PeerSendGetblocks(BRPeer *peer, const UInt256 *locators, size_t locatorsCount, UInt256 hashStop) {
				GetBlocksMessage *message = static_cast<GetBlocksMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETBLOCKS).get());

				message->SendGetBlocks(peer, locators, locatorsCount, hashStop);
			}

			void PeerSendGetdata(BRPeer *peer, const UInt256 *txHashes, size_t txCount,
								 const UInt256 *blockHashes, size_t blockCount) {
				GetDataMessage *message = static_cast<GetDataMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETDATA).get());

				message->SendGetData(peer, txHashes, txCount, blockHashes, blockCount);
			}

			int PeerAcceptGetData(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				GetDataMessage *message = static_cast<GetDataMessage *>(
						PeerMessageManager::instance().getMessage(MSG_GETDATA).get());

				return message->Accept(peer, msg, msgLen);
			}

			void PeerSendPingMessage(BRPeer *peer, void *info, void (*pongCallback)(void *info, int success)) {
				PingMessage *message = static_cast<PingMessage *>(
						PeerMessageManager::instance().getMessage(MSG_PING).get());

				message->sendPing(peer, info, pongCallback);
			}

			int PeerAcceptPingMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				PingMessage *message = static_cast<PingMessage *>(
						PeerMessageManager::instance().getMessage(MSG_PING).get());

				return message->Accept(peer, msg, msgLen);
			}

			int PeerAcceptPongMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
				PongMessage *message = static_cast<PongMessage *>(
						PeerMessageManager::instance().getMessage(MSG_PONG).get());
				
				return message->Accept(peer, msg, msgLen);
			}
		}

		PeerMessageManager PeerMessageManager::_instance = PeerMessageManager();

		PeerMessageManager::PeerMessageManager() {
		}

		PeerMessageManager::~PeerMessageManager() {
		}

		BRPeerMessages *PeerMessageManager::createMessageManager() {

			BRPeerMessages *peerMessages = BRPeerMessageNew();

			peerMessages->MerkleBlockNew = BRMerkleBlockNewWrapper;
			peerMessages->MerkleBlockFree = BRMerkleBlockFreeWrapper;
			peerMessages->ApplyFreeBlock = setApplyFreeBlock;

			peerMessages->BRPeerAcceptTxMessage = PeerAcceptTxMessage;
			peerMessages->BRPeerSendTxMessage = PeerSendTxMessage;
			_wrapperMessages[MSG_TX] = WrapperMessagePtr(new TransactionMessage);

			peerMessages->BRPeerAcceptMerkleblockMessage = PeerAcceptMerkleblockMessage;
			_wrapperMessages[MSG_MERKLEBLOCK] = WrapperMessagePtr(new MerkleBlockMessage);

			peerMessages->BRPeerAcceptVersionMessage = PeerAcceptVersionMessage;
			peerMessages->BRPeerSendVersionMessage = PeerSendVersionMessage;
			_messages[MSG_VERSION] = MessagePtr(new VersionMessage);

			peerMessages->BRPeerAcceptInventoryMessage = PeerAcceptInventoryMessage;
			peerMessages->BRPeerSendInventoryMessage = PeerSendInventoryMessage;
			_messages[MSG_INV] = MessagePtr(new InventoryMessage);

			peerMessages->BRPeerAcceptAddressMessage = PeerAcceptAddressMessage;
			peerMessages->BRPeerSendAddressMessage = PeerSendAddressMessage;
			_messages[MSG_ADDR] = MessagePtr(new AddressMessage);

			peerMessages->BRPeerAcceptNotFoundMessage = PeerAcceptNotFoundMessage;
			_messages[MSG_NOTFOUND] = MessagePtr(new NotFoundMessage);

			peerMessages->BRPeerSendFilterloadMessage = PeerSendFilterload;
			_wrapperMessages[MSG_FILTERLOAD] = WrapperMessagePtr(new BloomFilterMessage);

			peerMessages->BRPeerSendGetblocksMessage = PeerSendGetblocks;
			_messages[MSG_GETBLOCKS] = MessagePtr(new GetBlocksMessage);

			//use same message with getblocks
			peerMessages->BRPeerSendGetheadersMessage = PeerSendGetblocks;

			peerMessages->BRPeerSendGetdataMessage = PeerSendGetdata;
			peerMessages->BRPeerAcceptGetdataMessage = PeerAcceptGetData;
			_messages[MSG_GETDATA] = MessagePtr(new GetDataMessage);

			peerMessages->BRPeerSendPingMessage = PeerSendPingMessage;
			peerMessages->BRPeerAcceptPingMessage = PeerAcceptPingMessage;
			_messages[MSG_PING] = MessagePtr(new PingMessage);

			peerMessages->BRPeerAcceptPongMessage = PeerAcceptPongMessage;
			_messages[MSG_PONG] = MessagePtr(new PongMessage);

			return peerMessages;
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