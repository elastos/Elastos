// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTION_H__
#define __ELASTOS_SDK_TRANSACTION_H__

#include "BRTransaction.h"

#include "Wrapper.h"

namespace Elastos {
    namespace SDK {

        class Transaction :
            public Wrapper<BRTransaction *> {
        public:

            Transaction();

            Transaction(BRTransaction *transaction);

            ~Transaction();

            virtual std::string toString() const;

            virtual BRTransaction *getRaw();

            bool isRegistered() const;

            bool &isRegistered();

        private:
            bool _isRegistered;

            BRTransaction *_transaction;
        };

    }
}

#endif //__ELASTOS_SDK_TRANSACTION_H__
