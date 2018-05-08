// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRChainParams.h>
#include <BRMerkleBlock.h>

#include "arith_uint256.h"
#include "ChainParams.h"
#include "BRBCashParams.h"

namespace Elastos {
	namespace SDK {

		namespace {
			static int MainNetVerifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet) {
				return ChainParams::mainNet().verifyDifficulty(block, blockSet);
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
			_mainNet._targetTimespan = 60 * 2 * 720;
			_mainNet._targetTimePerBlock = 60 * 2;

			//todo init test net here
			_testNet._targetTimespan = 10 * 10;
			_testNet._targetTimePerBlock = 10 * 10;

			_paramsInit = false;
		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_chainParams = boost::shared_ptr<BRChainParams>(new BRChainParams(*params.getRaw()));
			return *this;
		}

		int ChainParams::verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet) const {
			const BRMerkleBlock *previous, *b = nullptr;
			uint32_t i;

			assert(block != nullptr);
			assert(blockSet != nullptr);

			uint64_t blocksPerRetarget = _targetTimespan / _targetTimePerBlock;

			// check if we hit a difficulty transition, and find previous transition block
			if ((block->height % blocksPerRetarget) == 0) {
				for (i = 0, b = block; b && i < blocksPerRetarget; i++) {
					b = (const BRMerkleBlock *) BRSetGet(blockSet, &b->prevBlock);
				}
			}

			previous = (const BRMerkleBlock *) BRSetGet(blockSet, &block->prevBlock);
			return verifyDifficaltyInner(block, previous, (b) ? b->timestamp : 0);
		}

		int ChainParams::verifyDifficaltyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous,
											   uint32_t transitionTime) const {
			int r = 1;

			assert(block != nullptr);
			assert(previous != nullptr);

			uint64_t blocksPerRetarget = _targetTimespan / _targetTimePerBlock;

			if (!previous || !UInt256Eq(&(block->prevBlock), &(previous->blockHash)) ||
				block->height != previous->height + 1)
				r = 0;
			if (r && (block->height % blocksPerRetarget) == 0 && transitionTime == 0) r = 0;

			if (r && (block->height % blocksPerRetarget) == 0) {
				uint32_t timespan = previous->timestamp - transitionTime;

				arith_uint256 target;
				target.SetCompact(previous->target);

				// limit difficulty transition to -75% or +400%
				if (timespan < _targetTimespan / 4) timespan = uint32_t(_targetTimespan) / 4;
				if (timespan > _targetTimespan * 4) timespan = uint32_t(_targetTimespan) * 4;

				// TARGET_TIMESPAN happens to be a multiple of 256, and since timespan is at least TARGET_TIMESPAN/4, we don't
				// lose precision when target is multiplied by timespan and then divided by TARGET_TIMESPAN/256
				target *= timespan;
				target /= _targetTimespan;

				uint32_t actualTargetCompact = target.GetCompact();
				if (block->target != actualTargetCompact) r = 0;
			} else if (r && previous->height != 0 && block->target != previous->target) r = 0;

			return r;
		}
	}
}
