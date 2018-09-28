// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Message.h"
#include "SDK/Wrapper/Peer.h"

namespace Elastos {
	namespace ElaWallet {

		SendMessageParameter Message::DefaultParam = SendMessageParameter();

		Message::Message(const MessagePeerPtr &peer) :
				_peer(peer) {

		}

		Message::~Message() {

		}

		void Message::FireConnected() {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnConnected();
		}

		void Message::FireDisconnected(int error) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnDisconnected(error);
		}

		void Message::FireRelayedPeers(const std::vector<MessagePeerPtr> &peers, size_t peersCount) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedPeers(peers, peersCount);
		}

		void Message::FireRelayedTx(const TransactionPtr &tx) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedTx(tx);
		}

		void Message::FireHasTx(const UInt256 &txHash) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnHasTx(txHash);
		}

		void Message::FireRejectedTx(const UInt256 &txHash, uint8_t code) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRejectedTx(txHash, code);
		}

		void Message::FireRelayedBlock(const MerkleBlockPtr &block) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedBlock(block);
		}

		void Message::FireRelayedPingMsg() {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedPingMsg();
		}

		void Message::FireNotfound(const std::vector<UInt256> &txHashes, const std::vector<UInt256> &blockHashes) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnNotfound(txHashes, blockHashes);
		}

		void Message::FireSetFeePerKb(uint64_t feePerKb) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnSetFeePerKb(feePerKb);
		}

		const TransactionPtr &Message::FireRequestedTx(const UInt256 &txHash) {
			if (_peer->_listener != nullptr)
				return _peer->_listener->OnRequestedTx(txHash);
			return nullptr;
		}

		bool Message::FireNetworkIsReachable() {
			if (_peer->_listener != nullptr)
				return _peer->_listener->OnNetworkIsReachable();
			return false;
		}

		void Message::FireThreadCleanup() {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnThreadCleanup();
		}

		void Message::SendMessage(const std::string &msg, const std::string &type) {
			//fixme [refactor] move implement from BRPeerSendMessage
		}
	}
}