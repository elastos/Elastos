// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_HEADERSMESSAGE_H__
#define __ELASTOS_SDK_HEADERSMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		class HeadersMessage : public Message {
		public:
			explicit HeadersMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const CMBlock &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};

	}
}


#endif //__ELASTOS_SDK_HEADERSMESSAGE_H__
