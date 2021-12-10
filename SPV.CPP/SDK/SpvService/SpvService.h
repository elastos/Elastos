// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSERVICE_H__
#define __ELASTOS_SDK_SPVSERVICE_H__

#include "Database/DatabaseManager.h"
#include "Config.h"

#include <Account/ISubAccount.h>
#include <Wallet/Wallet.h>

#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class DatabaseManager;
		class Transaction;

		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class SpvService {
		public:

			SpvService(const std::string &walletID,
					   const std::string &chainID,
					   const SubAccountPtr &subAccount,
					   const boost::filesystem::path &dbPath,
					   const ChainConfigPtr &config,
					   const std::string &netType);

			virtual ~SpvService();

			void DatabaseFlush();

            const WalletPtr &GetWallet() const;

		private:
			DatabaseManagerPtr _databaseManager;
            WalletPtr _wallet;
		};

	}
}

#endif //__ELASTOS_SDK_SPVSERVICE_H__
