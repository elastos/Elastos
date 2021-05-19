// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PONGMESSAGE_H__
#define __ELASTOS_SDK_PONGMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		struct PongParameter : public SendMessageParameter {
			uint64_t lastBlockHeight;

			PongParameter(uint64_t height) :
				lastBlockHeight(height)
			{}
		};

		class PongMessage :
				public Message {
		public:
			PongMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const bytes_t &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};

	}
}

#endif //__ELASTOS_SDK_PONGMESSAGE_H__
