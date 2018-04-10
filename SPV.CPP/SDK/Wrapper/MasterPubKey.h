// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MASTERPUBKEY_H__
#define __ELASTOS_SDK_MASTERPUBKEY_H__

#include <string>
#include <boost/shared_ptr.hpp>

#include "BRBIP32Sequence.h"

#include "Wrapper.h"

namespace Elastos {
    namespace SDK {

        class MasterPubKey :
            public Wrapper<BRMasterPubKey> {
        public:
            MasterPubKey();

            virtual std::string toString() const;

            virtual BRMasterPubKey getRaw();
        };

        typedef boost::shared_ptr<MasterPubKey> MasterPubKeyPtr;

    }
}

#endif //__ELASTOS_SDK_MASTERPUBKEY_H__
