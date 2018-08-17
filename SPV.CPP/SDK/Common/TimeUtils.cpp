// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <SDK/Wrapper/ChainParams.h>
#include <Core/BRChainParams.h>

#include "TimeUtils.h"
#include "Core/BRChainParams.h"

namespace Elastos {
	namespace ElaWallet {

		uint64_t TimeUtils::getCurrentTime() {
			time_t now = time(nullptr);
			return now;
		}

		std::string TimeUtils::convertToString(const char* pszFormat, uint64_t utcTime) {
			static std::locale classic(std::locale::classic());
			// std::locale takes ownership of the pointer
			std::locale loc(classic, new boost::posix_time::time_facet(pszFormat));
			std::stringstream ss;
			ss.imbue(loc);
			ss << boost::posix_time::from_time_t(utcTime);
			return ss.str();
		}

		uint64_t TimeUtils::calculateBlockHeightByTime(uint64_t utcTime, const ChainParams &chainParam) {
			size_t checkPointCount = chainParam.getRaw()->checkpointsCount;
			if (checkPointCount == 0  || chainParam.getRaw()->checkpoints[checkPointCount - 1].timestamp > utcTime)
				return 0;
			uint64_t timespan = utcTime - chainParam.getRaw()->checkpoints[checkPointCount - 1].timestamp;
			return timespan / chainParam.getTargetTimePerBlock();
		}
	}
}