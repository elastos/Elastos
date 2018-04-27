// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PONGMESSAGE_H__
#define __ELASTOS_SDK_PONGMESSAGE_H__

#include "IMessage.h"

namespace Elastos {
	namespace SDK {

		class PongMessage :
			public IMessage {
		public:
			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen);

			virtual void Send(BRPeer *peer);
		};

	}
}

#endif //__ELASTOS_SDK_PONGMESSAGE_H__
