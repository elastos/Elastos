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

            WrapperList() {
            }

            WrapperList(size_t count) :
                std::vector<T>(count) {
            }

            std::vector<Raw> getRawArray() const {

                std::vector<Raw> results;
                for(typename WrapperList<T, Raw>::const_iterator it = this->cbegin(); it != this->cend(); ++it) {
                    results.push_back(it->getRaw());
                }
                return results;
            }
        };

    }
}

#endif //__ELASTOS_SDK_OBJECTLIST_H__
