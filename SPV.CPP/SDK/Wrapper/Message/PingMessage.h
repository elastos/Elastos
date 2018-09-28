// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PINGMESSAGE_H__
#define __ELASTOS_SDK_PINGMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		class PingMessage :
			public Message {
		public:
			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen);

			virtual void Send(BRPeer *peer);

			void sendPing(BRPeer *peer, void *info, void (*pongCallback)(void *info, int success));
		};

	}
}

#endif //__ELASTOS_SDK_PINGMESSAGE_H__
