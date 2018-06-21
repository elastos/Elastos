// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMESSAGEMANAGER_H__
#define __ELASTOS_SDK_PEERMESSAGEMANAGER_H__

#include <unordered_map>
#include <boost/shared_ptr.hpp>

#include "BRPeerManager.h"

#include "IMessage.h"
#include "IWrapperMessage.h"

namespace Elastos {
	namespace ElaWallet {

		class PeerMessageManager {
		public:
			~PeerMessageManager();

			static PeerMessageManager& instance();

			typedef boost::shared_ptr<IMessage> MessagePtr;
			const MessagePtr &getMessage(const std::string &message);

			typedef boost::shared_ptr<IWrapperMessage> WrapperMessagePtr;
			const WrapperMessagePtr &getWrapperMessage(const std::string &message);

			BRPeerMessages *createMessageManager();

		private:
			PeerMessageManager();

		private:
			static PeerMessageManager _instance;

			std::unordered_map<std::string, MessagePtr> _messages;
			std::unordered_map<std::string, WrapperMessagePtr> _wrapperMessages;
		};

	}
}

#endif //SPVSDK_PEERMESSAGEMANAGER_H
