// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_VERSIONMESSAGE_H_
#define __ELASTOS_SDK_VERSIONMESSAGE_H_

#include "Message.h"

#include <SDK/P2P/Peer.h>

namespace Elastos {
	namespace ElaWallet {

		class VersionMessage :
				public Message {
		public:
			VersionMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const CMBlock &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;

		};

	}
}


#endif //__ELASTOS_SDK_VERSIONMESSAGE_H_
