// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IWRAPPERMESSAGE_H__
#define __ELASTOS_SDK_IWRAPPERMESSAGE_H__

#include "BRPeer.h"

#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace SDK {

		class IWrapperMessage {
		public:
			IWrapperMessage();

			virtual ~IWrapperMessage();

			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen, ELAMessageSerializable *serializable) = 0;

			virtual void Send(BRPeer *peer, ELAMessageSerializable *serializable) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IWRAPPERMESSAGE_H__
