// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>

#include "CoinConfig.h"

namespace Elastos {
	namespace SDK {

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
	}
}