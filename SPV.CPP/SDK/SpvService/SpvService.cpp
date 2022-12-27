// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "SpvService.h"

#include <Common/Utils.h>
#include <Plugin/Transaction/Transaction.h>
#include <Database/DatabaseManager.h>
#include <Wallet/WalletCommon.h>
#include <Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

		SpvService::SpvService(const std::string &walletID,
							   const std::string &chainID,
							   const SubAccountPtr &subAccount,
							   const boost::filesystem::path &dbPath,
							   const ChainConfigPtr &config,
							   const std::string &netType) :
				_databaseManager(new DatabaseManager(dbPath)) {

            if (chainID != CHAINID_MAINCHAIN &&
                chainID != CHAINID_IDCHAIN &&
                chainID != CHAINID_TOKENCHAIN) {
                ErrorChecker::ThrowParamException(Error::InvalidChainID, "invalid chain ID");
            }

            if (_wallet == nullptr) {
                _wallet = WalletPtr(new Wallet(walletID, chainID, subAccount, _databaseManager));
            }
		}

		SpvService::~SpvService() {
		}

		void SpvService::DatabaseFlush() {
			_databaseManager->flush();
		}

        const WalletPtr &SpvService::GetWallet() const {
            return _wallet;
        }

	}
}
