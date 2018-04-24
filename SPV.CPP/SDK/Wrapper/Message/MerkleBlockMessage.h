// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCKMESSAGE_H__
#define __ELASTOS_SDK_MERKLEBLOCKMESSAGE_H__

#include "IWrapperMessage.h"

namespace Elastos {
	namespace SDK {

		class MerkleBlockMessage :
			public IWrapperMessage {
		public:
			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen);

			virtual void Send(BRPeer *peer, void *serializable);
		};

	}
}

#endif //__ELASTOS_SDK_MERKLEBLOCKMESSAGE_H__
