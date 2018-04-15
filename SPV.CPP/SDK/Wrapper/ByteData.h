// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BYTEDATA_H__
#define __ELASTOS_SDK_BYTEDATA_H__

#include <cinttypes>
#include <cstddef>

namespace Elastos {
    namespace SDK {

        struct ByteData {
            ByteData() :
                    data(nullptr),
                    length(0) {
            }

            ByteData(uint8_t *script, size_t scriptLength) :
                    data(script),
                    length(scriptLength) {
            }

            uint8_t *data;
            size_t length;
        };

    }
}

#endif //__SPVCLIENT_BYTEDATA_H__
