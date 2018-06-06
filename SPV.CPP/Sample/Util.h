// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef __SPVCLIENT_UTIL_H__
#define __SPVCLIENT_UTIL_H__


#include <string>
#include <boost/shared_ptr.hpp>

#include "TWallet.h"

namespace Elastos {
    namespace APP {

        class Util {
        public:
            static bool selectAddress(boost::shared_ptr<TWallet> twallet, std::string& address);
        };

    }
}
#endif //SPVCLIENT_UTIL_H
