// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMESSAGEMANAGER_H__
#define __ELASTOS_SDK_PEERMESSAGEMANAGER_H__

#include <unordered_map>
#include <boost/shared_ptr.hpp>

#include "BRPeerManager.h"

#include "IMessage.h"

namespace Elastos {
	namespace SDK {

		class PeerMessageManager {
		public:
			~PeerMessageManager();

			static PeerMessageManager& instance();

			typedef boost::shared_ptr<IMessage> MessagePtr;
			const MessagePtr &getMessage(const std::string &message);

			void initMessages(BRPeerManager *peerManager);

		private:
			PeerMessageManager();

		private:
			static PeerMessageManager _instance;

			std::unordered_map<std::string, MessagePtr> _messages;
		};

	}
}

#endif //SPVSDK_PEERMESSAGEMANAGER_H
