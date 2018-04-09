// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEER_H__
#define __ELASTOS_SDK_PEER_H__

#include <boost/shared_ptr.hpp>

namespace Elastos {
    namespace SDK {

        class Peer {
        public:
        enum ConnectStatus {
            Disconnected = 0,
            Connecting = 1,
            Connected = 2,
            Unknown = -2
        };

        struct Status {
            Status(int v) {
                value = v;
            }

            int value;

            int getValue() {
                return value;
            }

            static ConnectStatus fromValue(int value) {
                for (int i = 0; i <= 2; i++) {
                    if (i == value)
                        return ConnectStatus(value);
                }
                return Unknown;
            }

            static std::string toString(ConnectStatus status) {
            //todo complete me
            }
        };


        public:
        //todo complete me
        };

        typedef boost::shared_ptr<Peer> PeerPtr;

    }
}

#endif //__ELASTOS_SDK_PEER_H__
