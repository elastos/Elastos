// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>

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
				config.TargetTimeSpan = it.value()["TargetTimeSpan"].get<uint32_t>();
				config.TargetTimePerBlock = it.value()["TargetTimePerBlock"].get<uint32_t>();
				config.StandardPort = it.value()["StandardPort"].get<uint16_t>();
				config.MagicNumber = it.value()["MagicNumber"].get<uint32_t>();
				config.Services = it.value()["Services"].get<uint32_t>();
				config.MinFee = it.value()["MinFee"].get<uint64_t>();
				std::vector<nlohmann::json> checkPoints = it.value()["CheckPoints"];
				for (int k = 0; k < checkPoints.size(); ++k) {
					config.CheckPoints.push_back(checkPoints[k]);
				}
				_configMap[config.ChainId] = config;
			}

			_initialized = true;
		}

		const CoinConfig &CoinConfigReader::FindConfig(const std::string &chainId) {
			if (_configMap.find(chainId) == _configMap.end()) {
				//todo instead of throw, we return default config for testing.
				return _configMap["ELA"];
			}
			return _configMap[chainId];
		}

		std::vector<std::string> CoinConfigReader::GetAllChainId() const {
			std::vector<std::string> result;
			std::for_each(_configMap.begin(), _configMap.end(), [&result](const CoinConfigMap::value_type &item){
				result.push_back(item.first);
			});
			return result;
		}
	}
}