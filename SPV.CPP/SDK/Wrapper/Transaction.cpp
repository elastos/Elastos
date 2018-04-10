// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Transaction.h"

namespace Elastos {
    namespace SDK {

        Transaction::Transaction(BRTransaction *transaction) :
            _transaction(transaction),
            _isRegistered(false) {
        }

        Transaction::~Transaction() {
            if (_transaction != nullptr)
                BRTransactionFree(_transaction);
        }

        std::string Transaction::toString() const {
            //todo complete me
            return "";
        }

        BRTransaction *Transaction::getRaw() const {
            //todo complete me
            return nullptr;
        }

        bool Transaction::isRegistered() const {
            return _isRegistered;
        }

        bool &Transaction::isRegistered() {
            return _isRegistered;
        }

    }
}
