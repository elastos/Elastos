// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_GETBLOCKSMESSAGE_H__
#define __ELASTOS_SDK_GETBLOCKSMESSAGE_H__

#include "IMessage.h"

namespace Elastos {
	namespace ElaWallet {

		class GetBlocksMessage :
				public IMessage {
		public:
			virtual int Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen);

			virtual void Send(BRPeer *peer);

			void SendGetBlocks(BRPeer *peer, const UInt256 locators[], size_t locatorsCount, UInt256 hashStop);
		};

	}
}

#endif //__ELASTOS_SDK_GETBLOCKSMESSAGE_H__
