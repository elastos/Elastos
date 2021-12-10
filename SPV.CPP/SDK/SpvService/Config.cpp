// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Config.h"

#include <Common/Log.h>

namespace Elastos {
    namespace ElaWallet {

        ChainConfig::ChainConfig() :
                _chainId(0),
                _networkId(0) {
        }

        const std::string &ChainConfig::Name() const {
            return _name;
        }

        int ChainConfig::ChainID() const {
            return _chainId;
        }

        int ChainConfig::NetworkID() const {
            return _networkId;
        }

        Config::Config(const Config &cfg) {
            this->operator=(cfg);
        }

        Config::Config(const std::string &netType, const nlohmann::json &config) {
            _netType = netType;
            if (_netType != CONFIG_MAINNET && _netType != CONFIG_TESTNET &&
                _netType != CONFIG_REGTEST && _netType != CONFIG_PRVNET) {
                Log::error("invalid NetType: {} in config json", _netType);
            }

            FromJSON(config);
        }

        Config::~Config() {

        }

        Config& Config::operator=(const Config &cfg) {
            this->_netType = cfg._netType;
            this->_chains = cfg._chains;

            return *this;
        }

        ChainConfigPtr Config::GetChainConfig(const std::string &id) const {
            if (_chains.find(id) == _chains.end())
                return nullptr;

            return _chains.at(id);
        }

        bool Config::FromJSON(const nlohmann::json &j) {
            try {
                for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
                    if (it.key() == "NetType")
                        continue;

                    std::string chainID = it.key();

                    ChainConfigPtr chainConfig(new ChainConfig());
                    if (chainID.find("ETH") != std::string::npos) {
                        nlohmann::json chainConfigJson = it.value();
                        chainConfig->_name = chainID + "-" + _netType;

                        chainConfig->_chainId = chainConfigJson["ChainID"].get<int>();
                        chainConfig->_networkId = chainConfigJson["NetworkID"].get<int>();
                    }

                    _chains[chainID] = chainConfig;
                }

                return true;
            } catch (const nlohmann::json::exception &e) {
                Log::error("config json format error: {}", e.what());
            }

            return false;
        }

        std::vector<std::string> Config::GetAllChainIDs() const {
            std::vector<std::string> result;

            std::for_each(_chains.begin(), _chains.end(),
                          [&result](const std::map<std::string, ChainConfigPtr>::value_type &item) {
                              result.push_back(item.first);
                          });

            return result;
        }

        std::string Config::GetNetType() const {
            return _netType;
        }

        const std::map<std::string, ChainConfigPtr> Config::GetConfigs() const {
            return _chains;
        }

    }
}