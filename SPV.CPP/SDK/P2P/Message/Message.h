// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MESSAGE_H__
#define __ELASTOS_SDK_MESSAGE_H__

#include <P2P/PeerInfo.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Interface/IMerkleBlock.h>

#include <string>
#include <boost/shared_ptr.hpp>

#define MSG_VERSION     "version"
#define MSG_VERACK      "verack"
#define MSG_ADDR        "addr"
#define MSG_INV         "inv"
#define MSG_GETDATA     "getdata"
#define MSG_NOTFOUND    "notfound"
#define MSG_GETBLOCKS   "getblocks"
#define MSG_GETHEADERS  "getheaders"
#define MSG_TX          "tx"
#define MSG_BLOCK       "block"
#define MSG_HEADERS     "headers"
#define MSG_GETADDR     "getaddr"
#define MSG_MEMPOOL     "mempool"
#define MSG_PING        "ping"
#define MSG_PONG        "pong"
#define MSG_FILTERLOAD  "filterload"
#define MSG_FILTERADD   "filteradd"
#define MSG_FILTERCLEAR "filterclear"
#define MSG_MERKLEBLOCK "merkleblock"
#define MSG_ALERT       "alert"
#define MSG_REJECT      "reject"   // described in BIP61: https://github.com/bitcoin/bips/blob/master/bip-0061.mediawiki
#define MSG_FEEFILTER   "feefilter"// described in BIP133 https://github.com/bitcoin/bips/blob/master/bip-0133.mediawiki

#define MAX_GETDATA_HASHES 50000

namespace Elastos {
	namespace ElaWallet {

		class Peer;

#define SUPPORT_NEW_INV_TYPE

		namespace {
			typedef enum {
				inv_undefined = 0,
				inv_tx = 1,
				inv_block = 2,
				inv_filtered_block = 3,
				inv_confirmed_block = 4,
				inv_address = 5,
				inv_filtered_sidechain_block = 6
			} inv_type;
		}

		typedef boost::shared_ptr<Peer> MessagePeerPtr;

		struct SendMessageParameter {
			virtual ~SendMessageParameter() {}
		};

		class Message {
		public:
			Message(const MessagePeerPtr &peer);

			virtual ~Message();

			virtual bool Accept(const bytes_t &msg) = 0;

			virtual void Send(const SendMessageParameter &param) = 0;

			virtual std::string Type() const = 0;

			static SendMessageParameter DefaultParam;

		protected:
			void FireConnected();

			void FireDisconnected(int error);

			void FireRelayedPeers(const std::vector<PeerInfo> &peers);

			void FireRelayedTx(const TransactionPtr &tx);

			void FireHasTx(const uint256 &txHash);

			void FireRejectedTx(const uint256 &txHash, uint8_t code, const std::string &reason);

			void FireRelayedBlock(const MerkleBlockPtr &block);

			void FireRelayedPing();

			void FireNotfound(const std::vector<uint256> &txHashes, const std::vector<uint256> &blockHashes);

			void FireSetFeePerKb(uint64_t feePerKb);

			TransactionPtr FireRequestedTx(const uint256 &txHash);

			bool FireNetworkIsReachable();

			void FireThreadCleanup();

			void SendMessage(const bytes_t &msg, const std::string &type);

		protected:
			MessagePeerPtr _peer;
		};

	}
}

#endif //__ELASTOS_SDK_MESSAGE_H__
