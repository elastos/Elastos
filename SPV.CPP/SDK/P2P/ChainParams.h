/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <Common/uint256.h>

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
