// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <SDK/Common/uint256.h>

#include <boost/shared_ptr.hpp>
#include <string>

namespace Elastos {
	namespace ElaWallet {

		class CheckPoint {
		public:
			CheckPoint() :
				_height(0),
				_timestamp(0),
				_target(0)
			{ }

			CheckPoint(uint32_t height, const std::string &hash, time_t timestamp, uint32_t target) :
				_height(height),
				_timestamp(timestamp),
				_target(target) {
				_hash.SetHex(hash);
			}

			CheckPoint(const CheckPoint &checkPoint) {
				this->operator=(checkPoint);
			}

			~CheckPoint() {}

			CheckPoint &operator=(const CheckPoint &checkpoint) {
				_height = checkpoint._height;
				_hash = checkpoint._hash;
				_timestamp = checkpoint._timestamp;
				_target = checkpoint._target;
				return *this;
			}

			const uint32_t &Height() const { return _height; }

			const uint256 &Hash() const { return _hash; }

			const time_t &Timestamp() const { return _timestamp; }

			const uint32_t &Target() const { return _target; }

		private:
			uint32_t _height;
			uint256 _hash;
			time_t _timestamp;
			uint32_t _target;
		};

		class ChainParams {
		public:
			ChainParams() :
				_standardPort(0),
				_magicNumber(0),
				_services(0),
				_targetTimeSpan(0),
				_targetTimePerBlock(0)
			{ }

			ChainParams(const ChainParams &chainParams) {
				this->operator=(chainParams);
			}

			~ChainParams() {}

			ChainParams &operator=(const ChainParams &params) {
				_dnsSeeds = params._dnsSeeds;
				_checkpoints = params._checkpoints;
				_standardPort = params._standardPort;
				_magicNumber = params._magicNumber;
				_services = params._services;
				_targetTimeSpan = params._targetTimeSpan;
				_targetTimePerBlock = params._targetTimePerBlock;
				return *this;
			}

			const std::vector<std::string> &DNSSeeds() const { return _dnsSeeds; }

			const CheckPoint &LastCheckpoint() const { return _checkpoints.back(); }

			const CheckPoint &FirstCheckpoint() const { return _checkpoints.front(); }

			const std::vector<CheckPoint> &Checkpoints() const { return _checkpoints; }

			const uint32_t &MagicNumber()  const { return _magicNumber; }

			const uint16_t &StandardPort() const { return _standardPort; }

			const uint64_t &Services() const { return _services; }

			const uint32_t &TargetTimeSpan() const { return _targetTimeSpan; }

			const uint32_t &TargetTimePerBlock() const { return _targetTimePerBlock; }

		private:
			friend class Config;

			std::vector<CheckPoint> _checkpoints;
			std::vector<std::string> _dnsSeeds;
			uint16_t _standardPort;
			uint32_t _magicNumber;
			uint64_t _services;

			uint32_t _targetTimeSpan;
			uint32_t _targetTimePerBlock;
		};

		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;

	}
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
