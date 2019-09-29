// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDTransaction.h"
#include "SDK/Plugin/Transaction/Payload/DIDInfo.h"
#include "SDK/Plugin/Transaction/Payload/RegisterIdentification.h"

namespace Elastos {
	namespace ElaWallet {

		IDTransaction::IDTransaction() : Transaction() {
		}

		IDTransaction::IDTransaction(uint8_t type, const PayloadPtr &payload) :
			Transaction(type, payload) {
		}

		IDTransaction::IDTransaction(const IDTransaction &tx) : Transaction(tx) {
		}

		IDTransaction &IDTransaction::operator=(const IDTransaction &tx) {
			Transaction::operator=(tx);
			return *this;
		}

		IDTransaction::~IDTransaction() {
		}

		PayloadPtr IDTransaction::InitPayload(uint8_t type) {
			PayloadPtr payload;

			if (registerIdentification == type) {
				payload = PayloadPtr(new RegisterIdentification());
			} else if (didTransaction == type) {
				payload = PayloadPtr(new DIDInfo());
			} else {
				payload = Transaction::InitPayload(type);
			}

			return payload;
		}

	}
}