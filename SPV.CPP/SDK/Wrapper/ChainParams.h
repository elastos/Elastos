// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CHAINPARAMS_H__
#define __ELASTOS_SDK_CHAINPARAMS_H__

#include <string>

#include "Wrapper.h"
#include "BRChainParams.h"

namespace Elastos {
    namespace SDK {

        class ChainParams :
            public Wrapper<BRChainParams>{
        public:
            ChainParams();

            virtual std::string toString() const;

            virtual BRChainParams *getRaw() const;
        };

    }
}

#endif //__ELASTOS_SDK_CHAINPARAMS_H__
