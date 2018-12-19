// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <SDK/Wrapper/Wrapper.h>
#include <SDK/KeyStore/CoinConfig.h>
#include <SDK/Wrapper/CheckPoint.h>

#include <boost/shared_ptr.hpp>
#include <string>

namespace Elastos {
	namespace ElaWallet {

		class ChainParams {
		public:
			ChainParams(const ChainParams &chainParams);

			ChainParams(const CoinConfig &config);

			ChainParams &operator=(const ChainParams &params);

			const std::vector<std::string> &GetDNSSeeds() const;

			const CheckPoint &GetLastCheckpoint() const;

			const CheckPoint &GetFirstCheckpoint() const;

			const std::vector<CheckPoint> &GetCheckpoints() const;

			const uint32_t &GetMagicNumber() const;

			const uint16_t &GetStandardPort() const;

			const uint64_t &GetServices() const;

			const uint32_t &GetTargetTimeSpan() const;

			const uint32_t &GetTargetTimePerBlock() const;

		private:
			void MainNetMainChainParamsInit();

			void MainNetIDChainParamsInit();

			void TestNetMainChainParamsInit();

			void TestNetIDChainParamsInit();

			void RegNetMainChainParamsInit();

			void RegNetIDChainParamsInit();

			void MainNetParamsInit(SubWalletType type);

			void TestNetParamsInit(SubWalletType type);

			void RegNetParamsInit(SubWalletType type);

		private:
			std::vector<std::string> _dnsSeeds;
			uint16_t _standardPort;
			uint32_t _magicNumber;
			uint64_t _services;
			std::vector<CheckPoint> _checkpoints;

			uint32_t _targetTimeSpan;
			uint32_t _targetTimePerBlock;
		};

		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;

	}
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
