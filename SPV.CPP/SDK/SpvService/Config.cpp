// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Config.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/P2P/ChainParams.h>

namespace Elastos {
	namespace ElaWallet {

#define CONFIG_FILENAME "Config.cfg"

		Config::Config() {
			libconfig::Config cfg;

			ErrorChecker::CheckPathExists(boost::filesystem::path(CONFIG_FILENAME));

			try {
				cfg.readFile(CONFIG_FILENAME);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowLogicException(Error::ReadConfigFileError, "read config file error: " +
																			  std::string(e.what()));
			}

			LoadConfig(cfg);
		}

		Config::Config(const std::string &path, const std::string &netType) {
			libconfig::Config cfg;

			cfg.setAutoConvert(true);

			boost::filesystem::path configPath = path;
			configPath /= CONFIG_FILENAME;

			ErrorChecker::CheckPathExists(configPath);

			try {
				cfg.readFile(configPath.string());
			} catch (const libconfig::ConfigException &e) {
				ErrorChecker::ThrowLogicException(Error::ReadConfigFileError, "read config file error: " +
																			  std::string(e.what()));
			}

			LoadConfig(cfg, netType);
		}

		Config::~Config() {

		}

		const ChainConfigPtr &Config::GetChainConfig(const std::string &id) const {
			for (size_t i = 0; i < _chains.size(); ++i) {
				if (_chains[i]->_id	 == id) {
					return _chains[i];
				}
			}

			return nullptr;
		}

		const std::vector<ChainConfigPtr> &Config::GetChainConfigs() const {
			return _chains;
		}

		void Config::LoadConfig(const libconfig::Config &cfg, const std::string &netType) {
			try {
				const libconfig::Setting &chainsSetting = cfg.getRoot()["Chains"];

				ErrorChecker::CheckLogic(!chainsSetting.isList() || chainsSetting.getLength() == 0,
										 Error::ReadConfigFileError, "Chains is empty or invalid");

				_chains.clear();
				for (int i = 0; i < chainsSetting.getLength(); ++i) {
					const libconfig::Setting &chainSetting = chainsSetting[i];
					ChainConfigPtr chainConfig(new ChainConfig());

					if (!(chainSetting.lookupValue("ID", chainConfig->_id) &&
						  chainSetting.lookupValue("Index", chainConfig->_index) &&
						  chainSetting.lookupValue("NetType", chainConfig->_netType) &&
						  chainSetting.lookupValue("MinFee", chainConfig->_minFee) &&
						  chainSetting.lookupValue("FeePerKB", chainConfig->_feePerKB) &&
						  chainSetting.lookupValue("DisconnectionTime", chainConfig->_disconnectionTime) &&
						  chainSetting.lookupValue("PluginType", chainConfig->_pluginType) &&
						  chainSetting.lookupValue("GenesisAddress", chainConfig->_genesisAddress))) {

						ErrorChecker::ThrowLogicException(Error::ReadConfigFileError, "invalid config");
					}
					if (netType.empty())
						chainConfig->_chainParameters = LoadChainParameter(chainSetting["ChainParameters"][chainConfig->NetType()]);
					else
						chainConfig->_chainParameters = LoadChainParameter(chainSetting["ChainParameters"][netType]);

					_chains.push_back(chainConfig);
				}
			} catch (const libconfig::SettingException &e) {
				ErrorChecker::ThrowLogicException(Error::ReadConfigFileError, "parse setting error: " +
																			  std::string(e.getPath()));
			}
		}

		ChainParamsPtr Config::LoadChainParameter(const libconfig::Setting &paramSetting) {
			ChainParamsPtr chainParams(new ChainParams());

			uint32_t port;
			if (!(paramSetting.lookupValue("StandardPort", port) &&
				  paramSetting.lookupValue("MagicNumber", chainParams->_magicNumber) &&
				  paramSetting.lookupValue("Services", chainParams->_services) &&
				  paramSetting.lookupValue("TargetTimeSpan", chainParams->_targetTimeSpan) &&
				  paramSetting.lookupValue("TargetTimePerBlock", chainParams->_targetTimePerBlock))) {

				ErrorChecker::ThrowLogicException(Error::ReadConfigFileError, "invalid config file");
			}

			chainParams->_standardPort = static_cast<uint16_t>(port);

			const libconfig::Setting &dnsSeed = paramSetting["DNSSeeds"];
			ErrorChecker::CheckLogic(!dnsSeed.isArray() || dnsSeed.getLength() == 0,
									 Error::ReadConfigFileError, "DNS seed is empty or invalid");

			for (int i = 0; i < dnsSeed.getLength(); ++i) {
				std::string seed = dnsSeed[i];
				chainParams->_dnsSeeds.push_back(seed);
			}

			const libconfig::Setting &checkpoints = paramSetting["CheckPoints"];
			ErrorChecker::CheckLogic(!checkpoints.isList() || checkpoints.getLength() == 0,
									 Error::ReadConfigFileError, "checkpoint is empty or invalid");

			for (int i = 0; i < checkpoints.getLength(); ++i) {
				uint32_t height = checkpoints[i][0];
				std::string hash = checkpoints[i][1];
				time_t timestamp = checkpoints[i][2];
				uint32_t target = checkpoints[i][3];

				chainParams->_checkpoints.emplace_back(height, hash, timestamp, target);
			}

			return chainParams;
		}

	}
}