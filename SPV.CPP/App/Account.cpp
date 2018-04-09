//
// Created by jzh on 18-4-9.
//

#include "Account.h"

using namespace std;
namespace po = boost::program_options;

namespace Elastos {
    namespace APP {

        bool Account::addAccount(boost::shared_ptr<TWallet> twallet, string publickey) {
            cout << "add account succeed" << endl;
            return true;
        }

        bool Account::deleteAccount(boost::shared_ptr<TWallet> twallet, string address) {
            cout << "delete account succeed" << endl;
            return true;
        }

        bool Account::addMultiSignAccount(boost::shared_ptr<TWallet> twallet, const po::variables_map& vm,
                                          std::string publickey) {
            cout << "add multiSign account succeed" << endl;
            return true;
        }

    }
}