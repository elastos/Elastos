// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include "MempoolMessage.h"
#include "SDK/P2P/Peer.h"

namespace Elastos {
	namespace ElaWallet {

		MempoolMessage::MempoolMessage(const MessagePeerPtr &peer) : Message(peer) {

		}

		bool MempoolMessage::Accept(const CMBlock &msg) {
			_peer->Pinfo("Drop mempool accept message, not implemented.");
			return false;
		}

		void MempoolMessage::Send(const SendMessageParameter &param) {
			const MempoolParameter &mempoolParameter = dynamic_cast<const MempoolParameter &>(param);

			struct timeval tv;
			bool sentMempool = _peer->SentMempool();
			_peer->SetSentMempool(true);

			if (!sentMempool && _peer->getMemPoolCallback().empty()) {
				_peer->AddKnownTxHashes(mempoolParameter.KnownTxHashes);

				if (!mempoolParameter.CompletionCallback.empty()) {
					gettimeofday(&tv, NULL);

					_peer->setMempoolTime(tv.tv_sec + (double)tv.tv_usec/1000000 + 10.0);
					_peer->setMempoolCallback(mempoolParameter.CompletionCallback);
				}

				_peer->SendMessage(CMBlock(), MSG_MEMPOOL);
			}
			else {
				_peer->Pinfo("mempool request already sent");
				if (mempoolParameter.CompletionCallback) mempoolParameter.CompletionCallback(0);
			}
		}

		std::string MempoolMessage::Type() const {
			return MSG_MEMPOOL;
		}
	}
}

