// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_GETADDRESSMESSAGE_H__
#define __ELASTOS_SDK_GETADDRESSMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		class GetAddressMessage : public Message {
		public:
			explicit GetAddressMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const bytes_t &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};

	}
}

#endif //__ELASTOS_SDK_GETADDRESSMESSAGE_H__
