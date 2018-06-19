// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_GETDATAMESSAGE_H__
#define __ELASTOS_SDK_GETDATAMESSAGE_H__

#include "IMessage.h"

namespace Elastos {
	namespace ElaWallet {

		class GetDataMessage :
			public IMessage {
		public:
			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen);

			virtual void Send(BRPeer *peer);

			virtual void SendGetData(BRPeer *peer, const UInt256 txHashes[],
									 size_t txCount, const UInt256 blockHashes[], size_t blockCount);
		};

	}
}

#endif //SPVSDK_GETDATAMESSAGE_H
