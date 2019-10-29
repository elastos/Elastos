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
			CheckPoint();

			CheckPoint(uint32_t height, const std::string &hash, time_t timestamp, uint32_t target);

			CheckPoint(const CheckPoint &checkPoint);

			~CheckPoint();

			CheckPoint &operator=(const CheckPoint &checkpoint);

			const uint32_t &Height() const;

			const uint256 &Hash() const;

			const time_t &Timestamp() const;

			const uint32_t &Target() const;

		private:
			uint32_t _height;
			uint256 _hash;
			time_t _timestamp;
			uint32_t _target;
		};

		class ChainParams {
		public:
			ChainParams();

			ChainParams(uint16_t standardPort, uint32_t magic,
				const std::vector<std::string> &dnsSeeds, const std::vector<CheckPoint> &checkpoints);

			ChainParams(const ChainParams &chainParams);

			~ChainParams();

			ChainParams &operator=(const ChainParams &params);

			const std::vector<std::string> &DNSSeeds() const;

			const CheckPoint &LastCheckpoint() const;

			const CheckPoint &FirstCheckpoint() const;

			const std::vector<CheckPoint> &Checkpoints() const;

			const uint32_t &MagicNumber() const;

			const uint16_t &StandardPort() const;

			const uint64_t &Services() const;

			const uint32_t &TargetTimeSpan() const;

			const uint32_t &TargetTimePerBlock() const;

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
