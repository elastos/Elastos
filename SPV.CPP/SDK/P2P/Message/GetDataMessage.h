// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_GETDATAMESSAGE_H__
#define __ELASTOS_SDK_GETDATAMESSAGE_H__

#include "Message.h"

namespace Elastos {
	namespace ElaWallet {

		struct GetDataParameter : public SendMessageParameter {
			std::vector<uint256> txHashes;
			std::vector<uint256> blockHashes;

			GetDataParameter(const std::vector<uint256> &txHashes, const std::vector<uint256> &blockHashes) :
				txHashes(txHashes), blockHashes(blockHashes) {

			}
		};

		class GetDataMessage : public Message {
		public:
			explicit GetDataMessage(const MessagePeerPtr &peer);

			virtual bool Accept(const bytes_t &msg);

			virtual void Send(const SendMessageParameter &param);

			virtual std::string Type() const;
		};

	}
}

#endif //SPVSDK_GETDATAMESSAGE_H
