// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>

#include "Enviroment.h"
#include "ParamChecker.h"
#include "MasterWalletManager.h"

namespace Elastos {
	namespace ElaWallet {

		std::string Enviroment::_rootPath = "";
		std::shared_ptr<IMasterWalletManager> Enviroment::_manager = nullptr;

		void Enviroment::InitializeRootPath(const std::string &rootPath) {
			ParamChecker::checkNotEmpty(rootPath);

			_rootPath = rootPath;
			CheckRootPath();
		}

		std::string Enviroment::GetRootPath() {
			return _rootPath;
		}

		IMasterWalletManager *Enviroment::GetMasterWalletManager() {
			CheckRootPath();

			if (_manager == nullptr)
				_manager = std::make_shared<MasterWalletManager>();
			return _manager.get();
		}

		void Enviroment::SaveConfigs() {
			if(_manager == nullptr)
				return;
			MasterWalletManager *manager = static_cast<MasterWalletManager *>(_manager.get());
			manager->SaveConfigs();
		}

		void Enviroment::CheckRootPath() {
			boost::filesystem::path path = _rootPath;
			if(!boost::filesystem::exists(path) || !boost::filesystem::is_directory(path))
				throw std::logic_error("Root path does not exist.");
		}

	}
}