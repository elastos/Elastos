// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadUpdateProducer.h"

#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadUpdateProducer::PayloadUpdateProducer() {

		}

		PayloadUpdateProducer::PayloadUpdateProducer(const PayloadUpdateProducer &payload) :
			PayloadRegisterProducer(payload) {
		}

		PayloadUpdateProducer::~PayloadUpdateProducer() {

		}

		PayloadUpdateProducer &PayloadUpdateProducer::operator=(const PayloadUpdateProducer &payload) {
			PayloadRegisterProducer::operator=(payload);
			return *this;
		}

	}
}
