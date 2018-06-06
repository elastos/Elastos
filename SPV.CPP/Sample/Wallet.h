// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __SPVCLIENT_WALLET_H__
#define __SPVCLIENT_WALLET_H__

#include <iostream>
#include <boost/program_options/variables_map.hpp>

#include "TWallet.h"

namespace Elastos {
    namespace APP {

        class Wallet {

        public:
            static bool walletAction(boost::program_options::variables_map vm);

        private:
            static bool createWallet(std::string name, std::string password);
            static bool changePassword(std::string name, std::string password);
            static bool showAccountInfo(std::string name, std::string password);
            static bool listBalanceInfo(boost::shared_ptr<TWallet> twallet);
            static bool getPassword(std::string password, bool confirmed);

        };

    }
}


#endif //SPVCLIENT_WALLET_H
