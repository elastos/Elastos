// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "Wrapper.h"
#include "BRChainParams.h"

namespace Elastos {
	namespace SDK {

		class ChainParams :
			public Wrapper<BRChainParams>{
		public:
			ChainParams(const BRChainParams &chainParams);

			virtual std::string toString() const;

			virtual BRChainParams *getRaw() const;

			uint32_t getMagicNumber() const;

		private:
			boost::shared_ptr<BRChainParams> _chainParams;

			const BRChainParams *_mainNetChainParams;
			const BRChainParams *_testNetChainParams;
			const BRChainParams *_mainNetBcashChainParams;
			const BRChainParams *_testNetBcashChainParams;
		};

	}
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
