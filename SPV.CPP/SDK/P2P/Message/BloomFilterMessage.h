// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BLOOMFILTERMESSAGE_H__
#define __ELASTOS_SDK_BLOOMFILTERMESSAGE_H__

#include "Message.h"
#include "SDK/Base/BloomFilter.h"

namespace Elastos {
	namespace ElaWallet {

		struct BloomFilterParameter : public SendMessageParameter {
			BloomFilterPtr Filter;
		};

		class BloomFilterMessage : public Message {
		public:
			explicit BloomFilterMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const CMBlock &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;

		};

	}
}

#endif //__ELASTOS_SDK_BLOOMFILTERMESSAGE_H__
