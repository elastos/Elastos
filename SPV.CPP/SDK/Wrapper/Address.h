// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESS_H__
#define __ELASTOS_SDK_ADDRESS_H__

#include <string>
#include <boost/shared_ptr.hpp>

namespace Elastos {
    namespace SDK {

        class Address {
        public:
            Address();

            std::string stringify() const;

            //todo complete me
        };

        typedef boost::shared_ptr<Address> AddressPtr;

    }
}

#endif //__ELASTOS_SDK_ADDRESS_H__
