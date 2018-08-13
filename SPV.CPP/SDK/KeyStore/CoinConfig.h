// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_COINCONFIG_H__
#define __ELASTOS_SDK_COINCONFIG_H__

#include <map>
#include <boost/filesystem.hpp>

#include <nlohmann/json.hpp>

#include "SubWalletType.h"
#include "Mstream.h"
#include "CheckPoint.h"

namespace Elastos {
	namespace ElaWallet {

		struct CoinConfig {
			std::string ChainId;
			SubWalletType Type;
			std::string NetType;
			uint32_t Index;
			uint32_t TargetTimeSpan;
			uint32_t TargetTimePerBlock;
			uint16_t StandardPort;
			uint32_t MagicNumber;
			uint32_t Services;
			uint64_t MinFee;
			std::string GenesisAddress;
			std::string BlockType;
			bool EnableP2P;
			std::vector<CheckPoint> CheckPoints;
		};

		/**
		 * This is a temporary for reading coin configs from config file.
		 * 	fixme when we can read coin config transactions in ela.
		 */
		class CoinConfigReader{
		public:
			CoinConfigReader();

			~CoinConfigReader();

			bool IsInitialized() const;

			void Load(const boost::filesystem::path &path);

			std::vector<std::string> GetAllChainId() const;

			const CoinConfig &FindConfig(const std::string &chainId);

		private:
			typedef std::map<std::string, CoinConfig> CoinConfigMap;
			CoinConfigMap _configMap;
			bool _initialized;
		};

	}
}

#endif //__ELASTOS_SDK_COINCONFIG_H__
