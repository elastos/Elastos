// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESSREGISTERINGWALLET_H__
#define __ELASTOS_SDK_ADDRESSREGISTERINGWALLET_H__

#include <vector>

#include "Wallet.h"

namespace Elastos {
	namespace SDK {

		class AddressRegisteringWallet : public Wallet {
		public:
			AddressRegisteringWallet(const boost::shared_ptr<Listener> &listener,
									 const std::vector<std::string> &initialAddrs);

			virtual ~AddressRegisteringWallet();

			virtual void RegisterAddress(const std::string &address, bool needSync);

		protected:
			BRWallet *createRegisterAddress(const std::vector<std::string> &initialAddrs);
		};

	}
}

#endif //__ELASTOS_SDK_ADDRESSREGISTERINGWALLET_H__
