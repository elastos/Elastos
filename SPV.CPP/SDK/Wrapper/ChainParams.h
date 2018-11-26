// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <SDK/Wrapper/Wrapper.h>
#include <SDK/KeyStore/CoinConfig.h>

#include <Core/BRChainParams.h>

#include <boost/shared_ptr.hpp>
#include <string>

namespace Elastos {
	namespace ElaWallet {

		struct ELAChainParams {
			BRChainParams Raw;

			//time unit is second
			uint32_t TargetTimeSpan;
			uint32_t TargetTimePerBlock;
			std::string NetType;
		};

		class ChainParams :
				public Wrapper<BRChainParams> {
		public:
			ChainParams(const ChainParams &chainParams);

			ChainParams(const CoinConfig &coinConfig);

			ChainParams &operator=(const ChainParams &params);

			virtual std::string toString() const;

			virtual BRChainParams *getRaw() const;

			uint32_t getMagicNumber() const;
			uint32_t getTargetTimeSpan() const;
			uint32_t getTargetTimePerBlock() const;

		private:
			void tryInit(const CoinConfig &coinConfig);

		private:

			boost::shared_ptr<ELAChainParams> _chainParams;
		};

	}
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
