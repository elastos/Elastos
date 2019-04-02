// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADUPDATEPRODUCER_H__
#define __ELASTOS_SDK_PAYLOADUPDATEPRODUCER_H__

#include <SDK/Plugin/Transaction/Payload/IPayload.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterProducer.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadUpdateProducer : public PayloadRegisterProducer {
		public:
			PayloadUpdateProducer();

			PayloadUpdateProducer(const PayloadUpdateProducer &payload);

			~PayloadUpdateProducer();

			PayloadUpdateProducer &operator=(const PayloadUpdateProducer &payload);

		};

	}
}

#endif //__ELASTOS_SDK_PAYLOADUPDATEPRODUCER_H__
