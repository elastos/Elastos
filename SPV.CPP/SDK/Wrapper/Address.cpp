// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"

namespace Elastos {
    namespace SDK {

        Address::Address() {
            //todo complete me
        }

        std::string Address::stringify() const {
            //todo complete me
            return "";
        }

        std::string Address::toString() const {
            return stringify();
        }

        BRAddress Address::getRaw() const {
            return BRAddress();
        }
    }
}