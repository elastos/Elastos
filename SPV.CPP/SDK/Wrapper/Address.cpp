// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRAddress.h>
#include "Address.h"

namespace Elastos {
    namespace SDK {
		Address::Address() {
			_address = boost::shared_ptr<BRAddress>(new BRAddress());
		}

		Address::Address(std::string address) {
			assert(!address.empty());

			_address = boost::shared_ptr<BRAddress>(new BRAddress());
			size_t stringLen = address.size();
			size_t stringLenMax = sizeof(_address->s) - 1;

			if (stringLen > stringLenMax) {
				stringLen = stringLenMax;
			}

			const char* stringChars = address.c_str();
			memcpy(_address->s, stringChars, stringLen);
		}

		Address::Address(const BRAddress &addr) {
			this->_address = boost::shared_ptr<BRAddress>(new BRAddress());
			memcpy(_address->s, addr.s, sizeof(_address->s));
		}

		boost::shared_ptr<Address> Address::createAddress(const std::string &address) {
			boost::shared_ptr<Address> addr = boost::shared_ptr<Address>(new Address(address));
			return addr;
		}

		boost::shared_ptr<Address> Address::fromScriptPubKey(ByteData script) {
			BRAddress address = {'\0'};

			BRAddressFromScriptPubKey(address.s, sizeof(address.s), script.data, script.length);

			boost::shared_ptr<Address> addr = boost::shared_ptr<Address>(new Address(address));

			return addr;
		}

		boost::shared_ptr<Address> Address::fromScriptSignature(ByteData script) {
			BRAddress address = {0};

			size_t scriptLen = script.length;
			BRAddressFromScriptSig(address.s, sizeof(address.s), script.data, scriptLen);

			boost::shared_ptr<Address> addr = boost::shared_ptr<Address>(new Address(address));

			return addr;
		}

		bool Address::isValid() {
			BRAddress* address = getRaw();
			bool res = BRAddressIsValid(address->s) ? true : false;
			return res;
		}

		ByteData Address::getPubKeyScript() {
			BRAddress* address = getRaw();

			size_t pubKeyLen = BRAddressScriptPubKey(NULL, 0, address->s);
			uint8_t *data = new uint8_t[pubKeyLen];
			BRAddressScriptPubKey(data, pubKeyLen, address->s);
			return ByteData(data, pubKeyLen);

		}

		std::string Address::stringify() const {
			BRAddress* address = getRaw();
			return address->s;
		}

		std::string Address::toString() const {
			return stringify();
		}

		BRAddress *Address::getRaw() const {
			return _address.get();
		}
    }
}