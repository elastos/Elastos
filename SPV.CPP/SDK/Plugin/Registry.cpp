// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Registry.h"
#include "ELAPlugin.h"
#include "IDPlugin.h"

namespace Elastos {
	namespace ElaWallet {

		Registry *Registry::Instance(bool erase) {
			static std::shared_ptr<Registry> instance(new Registry);
			if (erase) {
				instance.reset();
				instance = nullptr;
			}
			return instance.get();
		}

		MerkleBlockPtr Registry::CreateMerkleBlock(const std::string &pluginType) {
			if (_plugins.find(pluginType) == _plugins.end()) {
				if (_pluginInjectors.find(pluginType) == _pluginInjectors.end())
					return nullptr;

				std::vector<IPlugin *> plugins = _pluginInjectors[pluginType]->getMultibindings<IPlugin>();
				assert(!plugins.empty());
				_plugins[pluginType] = *plugins.begin();
			}
			return _plugins[pluginType]->CreateBlock();
		}

		Registry::Registry() {
		}

		void Registry::RegisterPlugin(const std::string &pluginType, fruit::Component<> (*pluginFun)()) {
			_pluginInjectors[pluginType] = PluginInjectorPtr(new fruit::Injector<>(pluginFun));
		}

		void Registry::UnRegisterPlugin(const std::string &pluginType) {
			if (_pluginInjectors.find(pluginType) != _pluginInjectors.end())
				_pluginInjectors.erase(pluginType);
		}

	}
}