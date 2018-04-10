// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WRAPPER_H__
#define __ELASTOS_SDK_WRAPPER_H__

#include <string>

namespace Elastos {
    namespace SDK {

        template <class T>
        class Wrapper {
        public:

            virtual std::string toString() const = 0;

            virtual T getRaw() const = 0;
        };

    }
}

#endif //__ELASTOS_SDK_WRAPPER_H__
