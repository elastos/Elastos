//
// Created by jzh on 18-4-9.
//

#include <iostream>

#include "TWallet.h"

using namespace std;

namespace Elastos {
    namespace APP {

        //this class is for test, will be removed after
        TWallet::TWallet() {

        }

        TWallet::~TWallet() {

        }

        bool TWallet::reset() {
            return false;
        }

        TxTransaction TWallet::createCrossCahainTransaction(std::string fromAddress, std::string toAddress,
                                                            std::string crossChainAddress, std::string amount,
                                                            std::string fee) {
            cout << "create cross chain transaction succeed !" << endl;
            return TxTransaction();
        }

        TxTransaction
        TWallet::createTransaction(std::string from, std::string to, std::string amount, std::string fee) {
            cout << "create standard transaction succeed !" << endl;
            return TxTransaction();
        }

        TxTransaction
        TWallet::createLockedTransaction(std::string from, std::string to, std::string amount, std::string fee) {
            cout << "create locked transaction succeed !" << endl;
            return TxTransaction();
        }

    }
}