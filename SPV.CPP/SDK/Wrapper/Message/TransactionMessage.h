// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONMESSAGE_H__
#define __ELASTOS_SDK_TRANSACTIONMESSAGE_H__

#include "IWrapperMessage.h"

namespace Elastos {
	namespace SDK {

		class TransactionMessage :
				public IWrapperMessage {
		public:
			TransactionMessage();

			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen, ELAMessageSerializable *serializable);

			virtual void Send(BRPeer *peer, ELAMessageSerializable *serializable);
		};

	}
}

#endif //__ELASTOS_SDK_TRANSACTIONMESSAGE_H__
