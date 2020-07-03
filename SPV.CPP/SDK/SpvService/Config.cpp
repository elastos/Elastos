// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Config.h"
#include "MainNetConfig.h"
#include "TestNetConfig.h"
#include "RegTestConfig.h"
#include "PrvNetConfig.h"

#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <P2P/ChainParams.h>
#include <Plugin/Registry.h>
#include <WalletCore/Address.h>

namespace Elastos {
	namespace ElaWallet {

		ChainConfig::ChainConfig() :
			_index(0),
			_minFee(0),
			_feePerKB(0),
			_disconnectionTime(0),
			_chainParameters(nullptr) {
		}

		const uint32_t &ChainConfig::Index() const {
			return _index;
		}

		const uint64_t &ChainConfig::MinFee() const {
			return _minFee;
		}

		const uint64_t &ChainConfig::FeePerKB() const {
			return _feePerKB;
		}

		const uint32_t &ChainConfig::DisconnectionTime() const {
			return _disconnectionTime;
		}

		const std::string &ChainConfig::GenesisAddress() const {
			return _genesisAddress;
		}

		const ChainParamsPtr &ChainConfig::ChainParameters() const {
			return _chainParameters;
		}

		Config::Config(const Config &cfg) {
			this->operator=(cfg);
		}

		Config::Config(const std::string &rootPath, const std::string &netType, const nlohmann::json &jsonConfig) :
			_filepath(rootPath + "/" + CONFIG_FILENAME) {
			ErrorChecker::CheckPathExists(boost::filesystem::path(rootPath));

			SetConfiguration(netType, jsonConfig);
			if (!Load())
				Log::error("load config failed");
		}

		Config::~Config() {

		}

		Config& Config::operator=(const Config &cfg) {
			this->_netType = cfg._netType;
			this->_filepath = cfg._filepath;
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
				if (j.find("NetType") == j.end()) {
					Log::error("NetType not found in config json");
					return false;
				}

				_netType = j["NetType"].get<std::string>();
				if (_netType != CONFIG_MAINNET && _netType != CONFIG_TESTNET &&
					_netType != CONFIG_REGTEST && _netType != CONFIG_PRVNET) {
					Log::error("invalid NetType: {} in config json", _netType);
					return false;
				}

				for (const std::string &chainID : supportChainIDList) {
					if (j.find(chainID) == j.end())
						continue;

					ChainConfigPtr chainConfig(new ChainConfig());
					nlohmann::json chainConfigJson = j[chainID];
					if (chainConfigJson.find("Index") != chainConfigJson.end())
						chainConfig->_index = chainConfigJson["Index"].get<uint32_t>();

					if (chainConfigJson.find("MinFee") != chainConfigJson.end())
						chainConfig->_minFee = chainConfigJson["MinFee"].get<uint64_t>();

					if (chainConfigJson.find("FeePerKB") != chainConfigJson.end())
						chainConfig->_feePerKB = chainConfigJson["FeePerKB"].get<uint64_t>();

					if (chainConfigJson.find("DisconnectionTime") != chainConfigJson.end())
						chainConfig->_disconnectionTime = chainConfigJson["DisconnectionTime"].get<uint32_t>();

					if (chainConfigJson.find("ChainParameters") != chainConfigJson.end()) {
						nlohmann::json chainParamsJson = chainConfigJson["ChainParameters"];
						ChainParamsPtr chainParams(new ChainParams());

						if (chainParamsJson.find("Services") != chainParamsJson.end())
							chainParams->_services = chainParamsJson["Services"].get<uint64_t>();

						if (chainParamsJson.find("MagicNumber") != chainParamsJson.end())
							chainParams->_magicNumber = chainParamsJson["MagicNumber"].get<uint32_t>();

						if (chainParamsJson.find("StandardPort") != chainParamsJson.end())
							chainParams->_standardPort = chainParamsJson["StandardPort"].get<uint16_t>();

						if (chainParamsJson.find("TargetTimeSpan") != chainParamsJson.end())
							chainParams->_targetTimeSpan = chainParamsJson["TargetTimeSpan"].get<uint32_t>();

						if (chainParamsJson.find("TargetTimePerBlock") != chainParamsJson.end())
							chainParams->_targetTimePerBlock = chainParamsJson["TargetTimePerBlock"].get<uint32_t>();

						if (chainParamsJson.find("DNSSeeds") != chainParamsJson.end())
							chainParams->_dnsSeeds = chainParamsJson["DNSSeeds"].get<std::vector<std::string>>();

						if (chainParamsJson.find("CheckPoints") != chainParamsJson.end()) {
							nlohmann::json checkPointsJosn = chainParamsJson["CheckPoints"];
							for (nlohmann::json::iterator it = checkPointsJosn.begin(); it != checkPointsJosn.end(); ++it) {
								uint32_t height = (*it)[0].get<uint32_t>();
								std::string hash = (*it)[1].get<std::string>();
								time_t timestamp = (*it)[2].get<time_t>();
								uint32_t target = (*it)[3].get<uint32_t>();
								chainParams->_checkpoints.emplace_back(height, hash, timestamp, target);
							}
						}

						uint256 hash = chainParams->_checkpoints[0].Hash();
						bytes_t data;
						data.push_back(hash.size());
						data.append(hash.bytes().data(), hash.size());
						data.push_back(SignTypeCrossChain); // The cross chain op code.

						Address address;
						address.SetRedeemScript(PrefixCrossChain, data);
						chainConfig->_genesisAddress = address.String();

						chainConfig->_chainParameters = chainParams;
						Log::debug("Lock address {}: {}", chainID, chainConfig->_genesisAddress);
					}
					_chains[chainID] = chainConfig;
				}

				return true;
			} catch (const nlohmann::json::exception &e) {
				Log::error("config json format error: {}", e.what());
			}

			return false;
		}

		bool Config::Load() {
			if (!boost::filesystem::exists(_filepath)) {
				Log::warn("{} do not exist", _filepath);
				return false;
			}

			std::ifstream ifs(_filepath);
			nlohmann::json j;
			ifs >> j;

			return FromJSON(j);
		}

		bool Config::ChangeConfig(nlohmann::json &currentConfig, const nlohmann::json &newConfig) const {
			bool changed = false;

			const std::vector<std::string> configNames = {"Index", "MinFee", "FeePerKB", "GenesisAddress",
														  "DisconnectionTime"};

			for (const std::string &configName : configNames) {
				if (newConfig.find(configName) != newConfig.end()) {
					currentConfig[configName] = newConfig[configName];
					changed = true;
				}
			}

			if (newConfig.find("ChainParameters") != newConfig.end()) {
				nlohmann::json newChainParameters = newConfig["ChainParameters"];
				nlohmann::json &currentChainParameters = currentConfig["ChainParameters"];

				const std::vector<std::string> chainParameterConfigNames = {
					"Services", "MagicNumber", "StandardPort", "TargetTimeSpan",
					"TargetTimePerBlock", "DNSSeeds", "CheckPoints"
				};

				for (const std::string &cpConfigName : chainParameterConfigNames) {
					if (newChainParameters.find(cpConfigName) != newChainParameters.end()) {
						currentChainParameters[cpConfigName] = newChainParameters[cpConfigName];
						changed = true;
					}
				}
			}

			return changed;
		}

		bool Config::SetConfiguration(const std::string &netType, const nlohmann::json &newConfig) {
			bool changed = false;
			nlohmann::json currentConfig;

			if (boost::filesystem::exists(_filepath)) {
				std::ifstream ifs(_filepath);
				ifs >> currentConfig;
			}

			if (netType == CONFIG_MAINNET) {
				currentConfig = DefaultMainNetConfig;
				changed = true;
			}

			if (currentConfig["NetType"] != netType) {
				if (netType == CONFIG_MAINNET) {
					currentConfig = DefaultMainNetConfig;
				} else if (netType == CONFIG_TESTNET) {
					currentConfig = DefaultTestNetConfig;
				} else if (netType == CONFIG_REGTEST) {
					currentConfig = DefaultRegTestConfig;
				} else if (netType == CONFIG_PRVNET) {
					currentConfig = DefaultPrvNetConfig;
				}
				currentConfig["NetType"] = netType;
				changed = true;
			}

			if (!newConfig.is_null()) {
				for (const std::string &chainID : supportChainIDList) {
					if (newConfig.find(chainID) != newConfig.end()) {
						if (ChangeConfig(currentConfig[chainID], newConfig[chainID])) {
							changed = true;
						}
					}
				}
			}

			if (changed) {
				std::ofstream ofs(_filepath);
				ofs << currentConfig.dump(4);
			}

			return changed;
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

	}
}