// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Registry.h"

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockPtr PluginHub::CreateMerkleBlock(const std::string &pluginType) {
			if (pluginType == "ELA")
				return _elaPlugin.get()->CreateBlock();
			else if (pluginType == "SideStandard")
				return _idPlugin.get()->CreateBlock();

			return nullptr;
		}

		fruit::Component<IPluginHub> getPluginHubComponent() {
			return fruit::createComponent()
					.bind<IPluginHub, PluginHub>()
					.install(getELAPluginComponent)
					.install(getIDPluginComponent);
		}

		Registry *Registry::Instance(bool erase) {
			static std::shared_ptr<Registry> instance(new Registry);
			if (erase) {
				instance.reset();
				instance = nullptr;
			}
			return instance.get();
		}

		MerkleBlockPtr Registry::CreateMerkleBlock(const std::string &blockType) {
			return _pluginHub->CreateMerkleBlock(blockType);
		}

		Registry::Registry() :
				_pluginHubInjector(getPluginHubComponent) {
			_pluginHub = _pluginHubInjector.get<IPluginHub *>();
		}

	}
}