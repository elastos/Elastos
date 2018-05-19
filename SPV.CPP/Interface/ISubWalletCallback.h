// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ISUBWALLETCALLBACK_H__
#define __ELASTOS_SDK_ISUBWALLETCALLBACK_H__

#include <string>

namespace Elastos {
	namespace SDK {

		class ISubWalletCallback {
		public:
			virtual void OnBalanceChanged(
					const std::string &address,
					double oldAmount,
					double newAmount) = 0;

			virtual void OnTransactionStatusChanged(
					const std::string &txid,
					const std::string &status,
					uint32_t error,
					const std::string &desc,
					uint32_t confirms) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ISUBWALLETCALLBACK_H__
