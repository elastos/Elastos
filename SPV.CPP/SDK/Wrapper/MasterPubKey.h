// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include <string>

namespace Elastos {
    namespace SDK {

        class MasterPubKey {
        public:
            MasterPubKey();

            std::string toString() const;
        };

    }
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
