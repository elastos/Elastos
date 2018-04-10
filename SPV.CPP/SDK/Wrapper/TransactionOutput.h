// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONOUTPUT_H__
#define __ELASTOS_SDK_TRANSACTIONOUTPUT_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "ByteData.h"
#include "BRTransaction.h"

namespace Elastos {
    namespace SDK {

        class TransactionOutput {
        public:
            TransactionOutput(uint64_t amount, const ByteData& script);

            std::string getAddress() const;

            uint64_t getAmount() const;

            void setAmount(uint64_t amount);

            ByteData getScript() const;

        private:
            boost::shared_ptr<BRTxOutput> _output;
        };
    }
}

#endif //__ELASTOS_SDK_TRANSACTIONOUTPUT_H__
