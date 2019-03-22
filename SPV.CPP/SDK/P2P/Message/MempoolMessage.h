// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__MEMPOOLMESSAGE_H__
#define __ELASTOS_SDK__MEMPOOLMESSAGE_H__

#include "Message.h"

#include <boost/function.hpp>

namespace Elastos {
	namespace ElaWallet {

		struct MempoolParameter : public SendMessageParameter {
			std::vector<uint256> KnownTxHashes;
			boost::function<void(int)> CompletionCallback;
		};

		class MempoolMessage : public Message {
		public:
			explicit MempoolMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const bytes_t &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};

	}
}

#endif //__ELASTOS_SDK__MEMPOOLMESSAGE_H__
