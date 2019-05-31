// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CONFIG_H__
#define __ELASTOS_SDK_CONFIG_H__

#include <libconfig.hh>

#include <boost/shared_ptr.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class ChainParams;

		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;

		class ChainConfig {
		public:
			ChainConfig() :
				_index(0),
				_minFee(0),
				_feePerKB(0),
				_disconnectionTime(0),
				_chainParameters(nullptr)
			{}

			const std::string &ID() const { return _id; }

			const uint32_t &Index() const { return _index; }

			const std::string &NetType() const { return _netType; }

			const uint64_t &MinFee() const { return _minFee; }

			const uint64_t &FeePerKB() const { return _feePerKB; }

			const uint32_t &DisconnectionTime() const { return _disconnectionTime; }

			const std::string &PluginType() const { return _pluginType; }

			const std::string &GenesisAddress() const { return _genesisAddress; }

			const ChainParamsPtr &ChainParameters() const { return _chainParameters; }

		private:
			friend class Config;

			std::string _id;
			uint32_t _index;
			std::string _netType;
			uint64_t _minFee;
			uint64_t _feePerKB;
			uint32_t _disconnectionTime;
			std::string _pluginType;
			std::string _genesisAddress;
			ChainParamsPtr _chainParameters;
		};

		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;


		class Config {
		public:
			Config();

			explicit Config(const std::string &path, const std::string &netType = "");

			~Config();

			const ChainConfigPtr &GetChainConfig(const std::string &id) const;

			const std::vector<ChainConfigPtr> &GetChainConfigs() const;

		private:
			void LoadConfig(const libconfig::Config &cfg, const std::string &netType = "");

			ChainParamsPtr LoadChainParameter(const libconfig::Setting &paramSetting);

		private:
			std::vector<ChainConfigPtr> _chains;
		};

		typedef boost::shared_ptr<Config> ConfigPtr;
	}
}


#endif //SPVSDK_CONFIG_H
