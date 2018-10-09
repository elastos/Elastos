// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MESSAGE_H__
#define __ELASTOS_SDK_MESSAGE_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "Transaction/Transaction.h"
#include "Plugin/Interface/IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class Peer;

		typedef boost::shared_ptr<Peer> MessagePeerPtr;

		struct SendMessageParameter {
		};

		class Message {
		public:
			Message(const MessagePeerPtr &peer);

			virtual ~Message();

			virtual bool Accept(const CMBlock &msg) = 0;

			virtual void Send(const SendMessageParameter &param) = 0;

			virtual std::string Type() const = 0;

			static SendMessageParameter DefaultParam;

		protected:
			void FireConnected();

			void FireDisconnected(int error);

			void FireRelayedPeers(const std::vector<MessagePeerPtr> &peers, size_t peersCount);

			void FireRelayedTx(const TransactionPtr &tx);

			void FireHasTx(const UInt256 &txHash);

			void FireRejectedTx(const UInt256 &txHash, uint8_t code);

			void FireRelayedBlock(const MerkleBlockPtr &block);

			void FireRelayedPingMsg();

			void FireNotfound(const std::vector<UInt256> &txHashes, const std::vector<UInt256> &blockHashes);

			void FireSetFeePerKb(uint64_t feePerKb);

			const TransactionPtr &FireRequestedTx(const UInt256 &txHash);

			bool FireNetworkIsReachable();

			void FireThreadCleanup();

			void SendMessage(const CMBlock &msg, const std::string &type);

		protected:
			MessagePeerPtr _peer;
		};

	}
}

#endif //__ELASTOS_SDK_MESSAGE_H__
