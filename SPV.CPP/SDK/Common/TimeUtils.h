// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TIMEUTILS_H__
#define __ELASTOS_SDK_TIMEUTILS_H__

#include <string>

#include "KeyStore/CoinConfig.h"
#include "ChainParams.h"

namespace Elastos {
	namespace ElaWallet {

		class TimeUtils {
		public:

			static uint64_t getCurrentTime();

			static std::string convertToString(const char* pszFormat, uint64_t utcTime);

			static uint64_t calculateBlockHeightByTime(uint64_t utcTime, const ChainParams &chainParams);
		};

	}
}


#endif //SPVSDK_TIMEUTILS_H
