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

		typedef std::string PluginType;

		class Registry : public boost::noncopyable {
		public:
			typedef boost::shared_ptr<fruit::Injector<>> PluginInjectorPtr;

			static Registry *Instance(bool erase = false);

			void RegisterPlugin(const std::string &pluginType, fruit::Component<> (*pluginFun)());

			void UnRegisterPlugin(const std::string &pluginType);

			MerkleBlockPtr CreateMerkleBlock(const std::string &pluginType);

		private:
			Registry();

			std::map<std::string, PluginInjectorPtr> _pluginInjectors;
			std::map<std::string, IPlugin *> _plugins;
		};


		class RegisterProxy {
		public:
			RegisterProxy(const std::string &pluginType, fruit::Component<> (*pluginFun)()) :
					_pluginType(pluginType) {
				if (Registry::Instance()) {
					Registry::Instance()->RegisterPlugin(pluginType, pluginFun);
				}
			}

			~RegisterProxy() {
				if (Registry::Instance()) {
					Registry::Instance()->UnRegisterPlugin(_pluginType);
				}
			}

		private:
			std::string _pluginType;
		};

#define REGISTER_MERKLEBLOCKPLUGIN(pluginKey, pluginComponentFunction) \
        static RegisterProxy g_plugin_proxy_##pluginKey = RegisterProxy(#pluginKey, pluginComponentFunction);

	}

}

#endif //__ELASTOS_SDK_REGISTRY_H__
