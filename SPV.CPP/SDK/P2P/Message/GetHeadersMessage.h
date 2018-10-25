// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_GETHEADERSMESSAGE_H__
#define __ELASTOS_SDK_GETHEADERSMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		struct GetHeadersParameter : public SendMessageParameter {
			std::vector<UInt256> locators;
			UInt256 hashStop;

			GetHeadersParameter() { memset(&hashStop, 0, sizeof(UInt256)); }
			GetHeadersParameter(const std::vector<UInt256> &locators, const UInt256 &hashStop) :
					locators(locators), hashStop(hashStop) {}
		};

		class GetHeadersMessage : public Message {
		public:
			explicit GetHeadersMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const CMBlock &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;

		};

	}
}

#endif //__ELASTOS_SDK_GETHEADERSMESSAGE_H__
