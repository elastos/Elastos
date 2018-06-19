// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __SPVSDK_SUBWALLETTYPE_H__
#define __SPVSDK_SUBWALLETTYPE_H__

namespace Elastos {
	namespace ElaWallet {

		enum SubWalletType {
			Normal = 0,
			Mainchain,
			Sidechain,
			Idchain
		};

	}
}

#endif //__SPVSDK_SUBWALLETTYPE_H__
