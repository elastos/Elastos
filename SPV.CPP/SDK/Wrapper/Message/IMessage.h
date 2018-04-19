// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "BRPeer.h"

#ifndef __ELASTOS_SDK_IMESSAGE_H__
#define __ELASTOS_SDK_IMESSAGE_H__

namespace Elastos {
	namespace SDK {

		class IMessage {
		public:
			IMessage();

			virtual ~IMessage();

			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) = 0;

			virtual void Send(BRPeer *peer) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IMESSAGE_H__
