// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
#include <SDK/Common/ParamChecker.h>

#include "CoinConfig.h"

namespace Elastos {
	namespace ElaWallet {

		namespace {
			SubWalletType convertToSubWalletType(const std::string &str) {
				if (str == "Mainchain")
					return Mainchain;
				else if (str == "Sidechain")
					return Sidechain;
				else if (str == "Idchain")
					return Idchain;
				else
					return Normal;
			}
		}

		CoinConfigReader::CoinConfigReader() : _initialized(false) {

		}

		CoinConfigReader::~CoinConfigReader() {

		}

		bool CoinConfigReader::IsInitialized() const {
			return _initialized;
		}

		void CoinConfigReader::Load(const boost::filesystem::path &path) {
			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
				CoinConfig config;
				config.ChainId = it.key();
				config.Type = convertToSubWalletType(it.value()["Type"].get<std::string>());
				config.Index = it.value()["CoinIndex"].get<uint32_t>();
				config.MinFee = it.value()["MinFee"].get<uint64_t>();
				config.BlockType = it.value()["PluginType"].get<std::string>();
				config.NetType = it.value()["NetType"].get<std::string>();
				config.GenesisAddress = it.value()["GenesisAddress"].get<std::string>();
				config.ReconnectSeconds = it.value()["ReconnectSeconds"].get<uint32_t>();
				config.EnableP2P = true;
				if (it.value().find("EnableP2P") != it.value().end())
					config.EnableP2P = it.value()["EnableP2P"].get<bool>();
				_configMap[config.ChainId] = config;
			}

			_initialized = true;
		}

		const CoinConfig &CoinConfigReader::FindConfig(const std::string &chainId) {
			ParamChecker::checkCondition(_configMap.find(chainId) == _configMap.end(), Error::IDNotFound,
										 "Chain id " + chainId + " not found");
			return _configMap[chainId];
		}

		std::vector<std::string> CoinConfigReader::GetAllChainId() const {
			std::vector<std::string> result;
			std::for_each(_configMap.begin(), _configMap.end(), [&result](const CoinConfigMap::value_type &item) {
				result.push_back(item.first);
			});
			return result;
		}

		std::map<std::string, uint32_t> CoinConfigReader::GetChainIdsAndIndices() const {
			std::map<std::string, uint32_t> result;
			std::for_each(_configMap.begin(), _configMap.end(), [&result](const CoinConfigMap::value_type &item) {
				result[item.first] = item.second.Index;
			});
			return result;
		}
	}
}