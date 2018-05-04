// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRChainParams.h>
#include "ChainParams.h"
#include "BRBCashParams.h"

namespace Elastos {
	namespace SDK {

		namespace {
			static int MainNetVerifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet)
			{
				return 1;
				//todo edit difficulty for ela node
				const BRMerkleBlock *previous, *b = NULL;
				uint32_t i;

				assert(block != NULL);
				assert(blockSet != NULL);

				// check if we hit a difficulty transition, and find previous transition block
				if ((block->height % BLOCK_DIFFICULTY_INTERVAL) == 0) {
					for (i = 0, b = block; b && i < BLOCK_DIFFICULTY_INTERVAL; i++) {
						b = (const BRMerkleBlock *)BRSetGet(blockSet, &b->prevBlock);
					}
				}

				previous = (const BRMerkleBlock *)BRSetGet(blockSet, &block->prevBlock);
				return BRMerkleBlockVerifyDifficulty(block, previous, (b) ? b->timestamp : 0);
			}

		}

		bool ChainParams::_paramsInit = false;
		ChainParams ChainParams::_mainNet = ChainParams(BRMainNetParams);
		ChainParams ChainParams::_testNet = ChainParams(BRTestNetParams);

		ChainParams::ChainParams() :
				_chainParams(nullptr) {
		}

		ChainParams::ChainParams(const ChainParams &chainParams) {
			_chainParams = boost::shared_ptr<BRChainParams>(new BRChainParams(*chainParams.getRaw()));
		}

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
			tryInit();
			return _mainNet;
		}

		const ChainParams &ChainParams::testNet() {
			tryInit();
			return _testNet;
		}

		void ChainParams::tryInit() {
			if (_paramsInit) return;

			_mainNet.getRaw()->standardPort = 10866;
			_mainNet.getRaw()->magicNumber = 7630401;
			_mainNet.getRaw()->checkpointsCount = 0;
			_mainNet.getRaw()->verifyDifficulty = MainNetVerifyDifficulty;

			//todo init test net here

			_paramsInit = false;
		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_chainParams = boost::shared_ptr<BRChainParams>(new BRChainParams(*params.getRaw()));
			return *this;
		}
	}
}
