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
				_peer->_listener->OnConnected(_peer.get());
		}

		void Message::FireDisconnected(int error) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnDisconnected(_peer.get(), error);
		}

		void Message::FireRelayedPeers(const std::vector<MessagePeerPtr> &peers, size_t peersCount) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedPeers(_peer.get(), peers, peersCount);
		}

		void Message::FireRelayedTx(const TransactionPtr &tx) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedTx(_peer.get(), tx);
		}

		void Message::FireHasTx(const UInt256 &txHash) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnHasTx(_peer.get(), txHash);
		}

		void Message::FireRejectedTx(const UInt256 &txHash, uint8_t code) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRejectedTx(_peer.get(), txHash, code);
		}

		void Message::FireRelayedBlock(const MerkleBlockPtr &block) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedBlock(_peer.get(), block);
		}

		void Message::FireRelayedPingMsg() {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnRelayedPingMsg(_peer.get());
		}

		void Message::FireNotfound(const std::vector<UInt256> &txHashes, const std::vector<UInt256> &blockHashes) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnNotfound(_peer.get(), txHashes, blockHashes);
		}

		void Message::FireSetFeePerKb(uint64_t feePerKb) {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnSetFeePerKb(_peer.get(), feePerKb);
		}

		const TransactionPtr &Message::FireRequestedTx(const UInt256 &txHash) {
			if (_peer->_listener != nullptr)
				return _peer->_listener->OnRequestedTx(_peer.get(), txHash);
			return nullptr;
		}

		bool Message::FireNetworkIsReachable() {
			if (_peer->_listener != nullptr)
				return _peer->_listener->OnNetworkIsReachable(_peer.get());
			return false;
		}

		void Message::FireThreadCleanup() {
			if (_peer->_listener != nullptr)
				_peer->_listener->OnThreadCleanup(_peer.get());
		}

		void Message::SendMessage(const CMBlock &msg, const std::string &type) {
			_peer->SendMessage(msg, type);
		}
	}
}