// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionOutput.h"

namespace Elastos {
    namespace SDK {

        TransactionOutput::TransactionOutput(uint64_t amount, const Script& script) {
            _output = boost::shared_ptr<BRTxOutput>(new BRTxOutput);
            BRTxOutputSetScript(_output.get(), script.data, script.length);
            _output->amount = amount;
        }

        std::string TransactionOutput::getAddress() const {
            return _output->address;
        }

        uint64_t TransactionOutput::getAmount() const {
            return _output->amount;
        }

        void TransactionOutput::setAmount(
                uint64_t amount) {
            _output->amount = amount;
        }

        Script TransactionOutput::getScript() const {
            return Script(_output->script, _output->scriptLen);
        }
    }
}