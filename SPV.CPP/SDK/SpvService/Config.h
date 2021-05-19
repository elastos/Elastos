// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CONFIG_H__
#define __ELASTOS_SDK_CONFIG_H__

#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

#define CONFIG_FILENAME "Config.json"
#define CONFIG_MAINNET "MainNet"
#define CONFIG_TESTNET "TestNet"
#define CONFIG_REGTEST "RegTest"
#define CONFIG_PRVNET  "PrvNet"

		class ChainParams;

		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;

		class ChainConfig {
		public:
			ChainConfig();

			const uint32_t &Index() const;

			const uint64_t &MinFee() const;

			const uint64_t &FeePerKB() const;

			const uint32_t &DisconnectionTime() const;

			const std::string &GenesisAddress() const;

			const ChainParamsPtr &ChainParameters() const;

		private:
			friend class Config;

			uint32_t _index;
			uint64_t _minFee;
			uint64_t _feePerKB;
			uint32_t _disconnectionTime;
			std::string _genesisAddress;
			ChainParamsPtr _chainParameters;
		};

		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;


		class Config {
		public:
			Config(const Config &cfg);

			Config(const std::string &rootPath, const std::string &netType = "MainNet",
				   const nlohmann::json &jsonConfig = nlohmann::json());

			~Config();

			Config &operator=(const Config &cfg);

			ChainConfigPtr GetChainConfig(const std::string &id) const;

			std::vector<std::string> GetAllChainIDs() const;

			std::string GetNetType() const;

		private:

			bool Load();

			bool SetConfiguration(const std::string &netType, const nlohmann::json &jsonConfig = nlohmann::json());

			bool FromJSON(const nlohmann::json &j);

			bool ChangeConfig(nlohmann::json &currentConfig, const nlohmann::json &newConfig) const;

		private:
			std::string _netType;
			std::map<std::string, ChainConfigPtr> _chains;
			std::string _filepath;
		};

		typedef boost::shared_ptr<Config> ConfigPtr;
	}
}


#endif //SPVSDK_CONFIG_H
