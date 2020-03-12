// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGISTRY_H__
#define __ELASTOS_SDK_REGISTRY_H__

#include "Interface/IPlugin.h"

#include <fruit/fruit.h>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include <memory>

namespace Elastos {
	namespace ElaWallet {

#define CHAINID_MAINCHAIN  "ELA"
#define CHAINID_IDCHAIN    "IDChain"
#define CHAINID_TOKENCHAIN "TokenChain"
#define CHAINID_ESC        "ETHSC"

		const std::vector<std::string> supportChainIDList = {CHAINID_MAINCHAIN, CHAINID_IDCHAIN, CHAINID_TOKENCHAIN, CHAINID_ESC};

		class Registry : public boost::noncopyable {
		public:
			typedef boost::shared_ptr<fruit::Injector<>> PluginInjectorPtr;

			static Registry *Instance(bool erase = false);

			void RegisterPlugin(const std::string &chainID, fruit::Component<> (*pluginFun)());

			void UnRegisterPlugin(const std::string &chainID);

			MerkleBlockPtr CreateMerkleBlock(const std::string &chainID);

		private:
			Registry();

			std::map<std::string, PluginInjectorPtr> _pluginInjectors;
			std::map<std::string, IPlugin *> _plugins;
		};


		class RegisterProxy {
		public:
			RegisterProxy(const std::string &chainID, fruit::Component<> (*pluginFun)()) :
					_chainID(chainID) {
				if (Registry::Instance()) {
					Registry::Instance()->RegisterPlugin(chainID, pluginFun);
				}
			}

			~RegisterProxy() {
				if (Registry::Instance()) {
					Registry::Instance()->UnRegisterPlugin(_chainID);
				}
			}

		private:
			std::string _chainID;
		};

#define REGISTER_MERKLEBLOCKPLUGIN(pluginKey, pluginComponentFunction) \
        static RegisterProxy g_plugin_proxy_##pluginKey = RegisterProxy(#pluginKey, pluginComponentFunction);

	}

}

#endif //__ELASTOS_SDK_REGISTRY_H__
