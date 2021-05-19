// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Registry.h"
#include "ELAPlugin.h"
#include "IDPlugin.h"
#include "TokenPlugin.h"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

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

		MerkleBlockPtr Registry::CreateMerkleBlock(const std::string &chainID) {
			if (_plugins.find(chainID) == _plugins.end()) {
				if (_pluginInjectors.find(chainID) == _pluginInjectors.end())
					return nullptr;

				std::vector<IPlugin *> plugins = _pluginInjectors[chainID]->getMultibindings<IPlugin>();
				assert(!plugins.empty());
				_plugins[chainID] = *plugins.begin();
			}
			return _plugins[chainID]->CreateBlock();
		}

		Registry::Registry() {
		}

		void Registry::RegisterPlugin(const std::string &chainID, fruit::Component<> (*pluginFun)()) {
			if (_pluginInjectors.find(chainID) != _pluginInjectors.end()) {
				return;
			}
			_pluginInjectors[chainID] = PluginInjectorPtr(new fruit::Injector<>(pluginFun));
		}

		void Registry::UnRegisterPlugin(const std::string &chainID) {
			if (_pluginInjectors.find(chainID) != _pluginInjectors.end())
				_pluginInjectors.erase(chainID);
		}

	}
}