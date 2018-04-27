// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ChainParams.h"
#include "BRBCashParams.h"

namespace Elastos {
	namespace SDK {

		ChainParams ChainParams::_mainNet = ChainParams(BRMainNetParams);
		ChainParams ChainParams::_testNet = ChainParams(BRTestNetParams);

		ChainParams::ChainParams(const BRChainParams &chainParams) {
			_chainParams = boost::shared_ptr<BRChainParams>(new BRChainParams(chainParams));
		}

		std::string ChainParams::toString() const {
			//todo complete me
			return "";
		}

		BRChainParams *ChainParams::getRaw() const {
			return _chainParams.get();
		}

		uint32_t ChainParams::getMagicNumber() const {
			return _chainParams->magicNumber;
		}

		const ChainParams &ChainParams::mainNet() {
			return _mainNet;
		}

		const ChainParams &ChainParams::testNet() {
			return _testNet;
		}

	}
}
