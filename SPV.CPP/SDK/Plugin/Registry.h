// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_REGISTRY_H__
#define __ELASTOS_SDK_REGISTRY_H__

#include <map>
#include <memory>
#include <fruit/fruit.h>
#include <boost/noncopyable.hpp>

#include "Interface/IPlugin.h"
#include "ELAPlugin.h"
#include "IDPlugin.h"

namespace Elastos {
	namespace ElaWallet {

		class IPluginHub {
		public:
			virtual ~IPluginHub() {}

			virtual MerkleBlockPtr CreateMerkleBlock(const std::string &pluginType) = 0;
		};

		class PluginHub : public IPluginHub {
		public:
			INJECT(PluginHub(
					ANNOTATED(ELAPluginTag, fruit::Provider<IPlugin>) elaPlugin,
					ANNOTATED(IDPluginTag, fruit::Provider<IPlugin>) idPlugin)) :
				_elaPlugin(elaPlugin),
				_idPlugin(idPlugin) {
			}

			virtual MerkleBlockPtr CreateMerkleBlock(const std::string &pluginType);

		private:
			fruit::Provider<IPlugin> _elaPlugin;
			fruit::Provider<IPlugin> _idPlugin;
		};

		fruit::Component<IPluginHub> getPluginHubComponent();


		class Registry : public boost::noncopyable {
		public:
			static Registry *Instance(bool erase = false);

			MerkleBlockPtr CreateMerkleBlock(const std::string &blockType);

		private:
			Registry();

			fruit::Injector<IPluginHub> _pluginHubInjector;
			IPluginHub *_pluginHub;
		};
	}

}

#endif //__ELASTOS_SDK_REGISTRY_H__
