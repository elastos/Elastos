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

#define CONFIG_MAINNET "MainNet"
#define CONFIG_TESTNET "TestNet"
#define CONFIG_REGTEST "RegTest"
#define CONFIG_PRVNET  "PrvNet"

        class ChainConfig {
        public:
            ChainConfig();

            const std::string &Name() const;

            int ChainID() const;

            int NetworkID() const;

        private:
            friend class Config;

            std::string _name;
            int _chainId;
            int _networkId;
        };

        typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;


        class Config {
        public:
            Config(const Config &cfg);

            Config(const std::string &netType = "MainNet", const nlohmann::json &config = nlohmann::json());

            ~Config();

            Config &operator=(const Config &cfg);

            ChainConfigPtr GetChainConfig(const std::string &id) const;

            std::vector<std::string> GetAllChainIDs() const;

            std::string GetNetType() const;

            const std::map<std::string, ChainConfigPtr> GetConfigs() const;

        private:
            bool FromJSON(const nlohmann::json &j);

        private:
            std::string _netType;
            std::map<std::string, ChainConfigPtr> _chains;
        };

        typedef boost::shared_ptr<Config> ConfigPtr;
    }
}


#endif //SPVSDK_CONFIG_H
