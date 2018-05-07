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
				public Wrapper<BRChainParams> {
		public:
			ChainParams();

			ChainParams(const ChainParams &chainParams);

			ChainParams &operator=(const ChainParams &params);

			virtual std::string toString() const;

			virtual BRChainParams *getRaw() const;

			uint32_t getMagicNumber() const;

			static const ChainParams &mainNet();

			static const ChainParams &testNet();

			int verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet) const;

		private:
			ChainParams(const BRChainParams &chainParams);

			static void tryInit();

			int verifyDifficaltyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous,
									  uint32_t transitionTime) const;

		private:
			//time unit is second
			uint64_t _targetTimespan;
			uint64_t _targetTimePerBlock;

			boost::shared_ptr<BRChainParams> _chainParams;

			static bool _paramsInit;
			static ChainParams _mainNet;
			static ChainParams _testNet;
		};

	}
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
