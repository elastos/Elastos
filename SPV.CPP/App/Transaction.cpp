//
// Created by jzh on 18-4-9.
//

#include "Util.h"
#include "Transaction.h"

using namespace std;
namespace po = boost::program_options;

namespace Elastos {
    namespace APP {

        bool Transaction::createTransaction(boost::shared_ptr<TWallet> twallet, const po::variables_map &vm) {
            if (!vm.count("fee")) {
                cout << "use --fee to specify transfer fee" << endl;
                return false;
            }

            string fee = vm["fee"].as<string>();
            string from = "";
            if (!vm.count("from")) {
                if (!Util::selectAddress(twallet, from)) {
                    return false;
                }
            }

            if (vm.count("file")) {
                string multiOutput = vm["file"].as<string>();
                return createMultiOutputTransaction(twallet, vm, multiOutput, from, fee);
            }

            if (!vm.count("amount")) {
                cout << "use --amount to specify transfer amount" << endl;
            }
            string amount = vm["amount"].as<string>();

            string to = "";
            TxTransaction txn;
            if (vm.count("deposit")) {
                to = "genesisAddress"; //TODO get genesis address from config
                txn = twallet->createCrossCahainTransaction(from, to, vm["deposit"].as<string>(), amount, fee);
            } else if (vm.count("withdraw")) {
                to = "destroyAddress"; //TODO get destroy address from config
                txn = twallet->createCrossCahainTransaction(from, to, vm["withdraw"].as<string>(), amount, fee);
            } else if (vm.count("to")) {
                to = vm["to"].as<string>();
                if (!vm.count("lock")) {
                    txn = twallet->createTransaction(from, to, amount, fee);
                }
                else {
                    //TODO parseUint(lockstr)
                    string lockTime = vm["lock"].as<string>();
                    txn = twallet->createLockedTransaction(from, to, amount, fee);
                }
            }
            else {
                cout << "use --to or --deposit or --withdraw to specify receiver address " << endl;
                return false;
            }

            //TODO output(haveSign, needSign, txn)

            return true;
        }

        bool Transaction::createMultiOutputTransaction(boost::shared_ptr<TWallet> twallet, const po::variables_map &vm,
                                                       string path, string from, string fee) {

            cout << "create multi output transaction succeed !" << endl;
            return true;
        }

        bool Transaction::signTransaction(boost::shared_ptr<TWallet> twallet, const po::variables_map &vm,
                                          string name, string password) {

            cout << "sign transaction succeed !" << endl;
            return true;
        }

        bool Transaction::sendTransaction(boost::shared_ptr<TWallet> twallet, const po::variables_map &vm) {

            cout << "send transction succeed !" << endl;
            return true;
        }

    }
}