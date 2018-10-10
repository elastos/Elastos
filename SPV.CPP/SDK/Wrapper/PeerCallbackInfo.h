// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERCALLBACKINFO_H__
#define __ELASTOS_SDK_PEERCALLBACKINFO_H__

#include <boost/shared_ptr.hpp>

#include "BRInt.h"

namespace Elastos {
	namespace ElaWallet {

		class Peer;
		class PeerManager;

		struct FindPeersInfo{
			PeerManager *manager;
			const char *hostname;
			uint64_t services;
		};

		struct PeerCallbackInfo{
			boost::shared_ptr<Peer> peer;
			PeerManager *manager;
			UInt256 hash;
		};
	}
}

#endif //__ELASTOS_SDK_PEERCALLBACKINFO_H__
