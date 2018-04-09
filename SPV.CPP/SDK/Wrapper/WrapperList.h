// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_OBJECTLIST_H__
#define __ELASTOS_SDK_OBJECTLIST_H__

#include <vector>

#include "Wrapper.h"

namespace Elastos {
    namespace SDK {

        template <class T, class Raw>
        class WrapperList :
            public std::vector<T> {

        public:

            std::vector<Raw> getRawArray() {

                std::vector<Raw> results;
                for(typename WrapperList<T, Raw>::iterator it = this->begin(); it != this->end(); ++it) {
                    results.push_back(it->getRaw());
                }
                return results;
            }
        };

    }
}

#endif //__ELASTOS_SDK_OBJECTLIST_H__
