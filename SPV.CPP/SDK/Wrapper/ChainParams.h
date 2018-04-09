// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <string>

namespace Elastos {
    namespace SDK {

        class ChainParams {
        public:
            ChainParams();

            std::string toString() const;
        };

    }
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
