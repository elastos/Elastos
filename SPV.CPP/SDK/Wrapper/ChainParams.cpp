// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRChainParams.h>
#include <BRMerkleBlock.h>
#include <Core/BRChainParams.h>

#include "ChainParams.h"
#include "BRBCashParams.h"

namespace Elastos {
	namespace ElaWallet {

		ChainParams::ChainParams(const ChainParams &chainParams) {
			_chainParams = boost::shared_ptr<ELAChainParams>(
					new ELAChainParams(*(ELAChainParams *) chainParams.getRaw()));
		}

		ChainParams::ChainParams(const CoinConfig &coinConfig) {
			_chainParams = boost::shared_ptr<ELAChainParams>(new ELAChainParams());
			tryInit(coinConfig);
		}

		std::string ChainParams::toString() const {
			//todo complete me
			return "";
		}

		BRChainParams *ChainParams::getRaw() const {
			return (BRChainParams *) _chainParams.get();
		}

		uint32_t ChainParams::getMagicNumber() const {
			return _chainParams->Raw.magicNumber;
		}

		void ChainParams::tryInit(const CoinConfig &coinConfig) {
			_chainParams->Raw.dnsSeeds = nullptr;
			_chainParams->Raw.standardPort = coinConfig.StandardPort;
			_chainParams->Raw.magicNumber = coinConfig.MagicNumber;
			_chainParams->Raw.checkpointsCount = coinConfig.CheckPoints.size();
			_checkPoints.clear();
			for (int i = 0; i < coinConfig.CheckPoints.size(); ++i) {
				_checkPoints.push_back(*coinConfig.CheckPoints[i].getRaw());
			}
			_chainParams->Raw.checkpoints = _checkPoints.data();
			_chainParams->Raw.verifyDifficulty = nullptr;
			_chainParams->TargetTimeSpan = coinConfig.TargetTimeSpan;
			_chainParams->TargetTimePerBlock = coinConfig.TargetTimePerBlock;
		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_chainParams = boost::shared_ptr<ELAChainParams>(new ELAChainParams(*(ELAChainParams *) params.getRaw()));
			return *this;
		}

	}
}
