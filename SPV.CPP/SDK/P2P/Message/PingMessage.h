// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PINGMESSAGE_H__
#define __ELASTOS_SDK_PINGMESSAGE_H__

#include "Message.h"

#include <boost/function.hpp>

namespace Elastos {
	namespace ElaWallet {

		struct PingParameter : public SendMessageParameter {
			boost::function<void (int)> callback;
			uint64_t lastBlockHeight;

			PingParameter(uint64_t height, boost::function<void (int)> cb) :
				lastBlockHeight(height),
				callback(cb)
			{}
		};

		class PingMessage :
			public Message {
		public:
			explicit PingMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const bytes_t &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};

	}
}

#endif //__ELASTOS_SDK_PINGMESSAGE_H__
