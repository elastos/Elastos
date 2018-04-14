//
// Created by jzh on 18-4-9.
//

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
