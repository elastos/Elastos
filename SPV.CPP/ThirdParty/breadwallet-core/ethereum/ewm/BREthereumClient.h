//
//  BREthereumClient.h
//  BRCore
//
//  Created by Ed Gamble on 11/20/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Client_H
#define BR_Ethereum_Client_H

#include "BREthereumBase.h"
#include "BRCryptoSync.h"

#ifdef __cplusplus
extern "C" {
#endif

    // Cookies are used as markers to match up an asynchronous operation
    // request with its corresponding event.
    typedef void *BREthereumCookie;

#ifdef __cplusplus
}
#endif

#endif //BR_Ethereum_Client_H

