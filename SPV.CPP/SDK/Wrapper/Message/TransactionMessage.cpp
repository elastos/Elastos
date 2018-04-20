// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionMessage.h"
#include "Transaction.h"

namespace Elastos {
	namespace SDK {

		TransactionMessage::TransactionMessage() {

		}

		int TransactionMessage::Accept(
				BRPeer *peer, const uint8_t *msg, size_t msgLen, ELAMessageSerializable *serializable) {

			Transaction *transaction = static_cast<Transaction *>(serializable);
			return 0;
		}

		void TransactionMessage::Send(BRPeer *peer, ELAMessageSerializable *serializable) {

		}
	}
}