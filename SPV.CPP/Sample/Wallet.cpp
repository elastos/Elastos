//
// Created by jzh on 18-4-9.
//

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "Wallet.h"
#include "Account.h"
#include "Transaction.h"

using namespace std;
namespace po = boost::program_options;

namespace Elastos {
    namespace APP {


        bool Wallet::walletAction(boost::program_options::variables_map vm) {
            if (vm.size() == 0) {
                std::cout << "no paramter input " << std::endl;
                return EXIT_FAILURE;
            }

            std::string name = "";
            std::string pass = "";
            if (vm.count("name")) {
                name = vm["name"].as<std::string>();
            }

            if (vm.count("password")) {
                pass = vm["password"].as<std::string>();
            }

            if (vm.count("create")) {
                return createWallet(name, pass);
            }

            //TODO openWallet
            boost::shared_ptr<TWallet> twallet = boost::make_shared<TWallet>();

            //show account information
            if (vm.count("account")) {
                return showAccountInfo(name, pass);
            }

            //change password
            if (vm.count("changepassword")) {
                return changePassword(name, pass);
            }

            //add account
            if (vm.count("addaccount")) {
                std::string pubKeyStr = vm["addaccount"].as<std::string>();
                return Account::addAccount(twallet, pubKeyStr);
            }

            //add addmultisignaccount
            if (vm.count("addmultisignaccount")) {
                std::string pubKeyStr = vm["addmultisignaccount"].as<std::string>();
                return Account::addMultiSignAccount(twallet, vm, pubKeyStr);
            }

            //delete account
            if (vm.count("deleteaccount")) {
                std::string address = vm["deleteaccount"].as<std::string>();
                return Account::deleteAccount(twallet, address);
            }

            //show balance information
            if (vm.count("balance")) {
                std::cout << "show balance infromation succeed" << std::endl;
                return listBalanceInfo(twallet);
            }

            //transaction actions
            if (vm.count("transaction")) {
                std::string txType = vm["transaction"].as<std::string>();
                if (txType == "create") {
                    return Transaction::createTransaction(twallet, vm);

                } else if (txType == "sign") {
                    return Transaction::signTransaction(twallet, vm, name, pass);

                } else if (txType == "send") {
                    return Transaction::sendTransaction(twallet, vm);
                }
            }

            if (vm.count("reset")) {
                return twallet->reset();
            }

            return EXIT_FAILURE;
        }

        bool Wallet::createWallet(std::string name, std::string password) {
            cout << "create wallet succeed !" << endl;
            return true;
        }

        bool Wallet::changePassword(std::string name, std::string password) {
            cout << "change password succeed !" << endl;
            return true;
        }

        bool Wallet::showAccountInfo(std::string name, std::string password) {
            cout << "show account information succeed !" << endl;
            return true;
        }

        bool Wallet::listBalanceInfo(boost::shared_ptr<TWallet> twallet) {
            cout << "list balance information succeed !" << endl;
            return true;
        }

    }
}