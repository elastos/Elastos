// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SHAREDOBJECTLIST_H__
#define __ELASTOS_SDK_SHAREDOBJECTLIST_H__

#include <vector>
#include <boost/shared_ptr.hpp>

#include "Wrapper.h"

namespace Elastos {
    namespace ElaWallet {

        template <class T, class Raw>
        class SharedWrapperList :
            public std::vector< boost::shared_ptr< T > > {
        public:
            typedef boost::shared_ptr<T> TPtr;

            SharedWrapperList() {
            }

            SharedWrapperList(size_t count) :
                    std::vector< boost::shared_ptr< T > >(count) {
            }

            std::vector<Raw> getRawPointerArray() const {

                std::vector<Raw> results;
                for(typename SharedWrapperList<T, Raw>::const_iterator it = this->cbegin(); it != this->cend(); ++it) {
                    results.push_back((*it)->getRaw());
                }
                return results;
            }
        };

    }
}

#endif //__ELASTOS_SDK_SHAREDOBJECTLIST_H__
