// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_NOTFOUNDMESSAGE_H_
#define __ELASTOS_SDK_NOTFOUNDMESSAGE_H_

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		class NotFoundMessage :
			public Message {
		public:
			NotFoundMessage(const MessagePeerPtr &peer);

			//fixme [refactor]
//			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen);
//
//			virtual void Send(BRPeer *peer);

		};

	}
}


#endif //__ELASTOS_SDK_NOTFOUNDMESSAGE_H_
