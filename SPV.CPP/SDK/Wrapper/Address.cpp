// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRAddress.h>
#include <Core/BRAddress.h>

#include "BRBase58.h"
#include "BRBech32.h"

#include "Address.h"

#define MAX_SCRIPT_LENGTH 0x100 // scripts over this size will not be parsed for an address

namespace Elastos {
    namespace ElaWallet {
		Address::Address() {
			_address = boost::shared_ptr<BRAddress>(new BRAddress());
		}

		Address::Address(std::string address) {
			_address = boost::shared_ptr<BRAddress>(new BRAddress());
			memset(_address->s, 0, sizeof(_address->s));
			strncpy(_address->s, address.c_str(), sizeof(_address->s) - 1);
		}

		Address::Address(const BRAddress &addr) {
			this->_address = boost::shared_ptr<BRAddress>(new BRAddress());
			memcpy(_address->s, addr.s, sizeof(_address->s));
		}

		boost::shared_ptr<Address> Address::createAddress(const std::string &address) {
			boost::shared_ptr<Address> addr = boost::shared_ptr<Address>(new Address(address));
			return addr;
		}

		boost::shared_ptr<Address> Address::fromScriptPubKey(CMBlock script) {
			BRAddress address = {'\0'};
			size_t scriptLen = script.GetSize(), addrLen = sizeof(address.s);

			if (! script || scriptLen == 0 || scriptLen > MAX_SCRIPT_LENGTH) {
				throw std::logic_error("fromScriptPubKey params error");
			}

			char a[91];
			uint8_t data[21];
			const uint8_t *d, *elems[BRScriptElements(NULL, 0, script, scriptLen)];
			size_t r = 0, l = 0, count = BRScriptElements(elems, sizeof(elems)/sizeof(*elems), script, scriptLen);

			if (count == 5 && *elems[0] == OP_DUP && *elems[1] == OP_HASH160 && *elems[2] == 20 &&
			    *elems[3] == OP_EQUALVERIFY && *elems[4] == OP_CHECKSIG) {
				// pay-to-pubkey-hash scriptPubKey
				data[0] = ELA_STAND_ADDRESS;
#if BITCOIN_TESTNET
				data[0] = ELA_STAND_ADDRESS;
#endif
				memcpy(&data[1], BRScriptData(elems[2], &l), 20);
				r = BRBase58CheckEncode(address.s, addrLen, data, 21);
			}
			else if (count == 3 && *elems[0] == OP_HASH160 && *elems[1] == 20 && *elems[2] == OP_EQUAL) {
				// pay-to-script-hash scriptPubKey
				data[0] = ELA_MULTISIG_ADDRESS;
#if BITCOIN_TESTNET
				data[0] = ELA_MULTISIG_ADDRESS;
#endif
				memcpy(&data[1], BRScriptData(elems[1], &l), 20);
				r = BRBase58CheckEncode(address.s, addrLen, data, 21);
			}
			else if (count == 2 && (*elems[0] == 65 || *elems[0] == 33) && *elems[1] == OP_CHECKSIG) {
				// pay-to-pubkey scriptPubKey
				data[0] = ELA_STAND_ADDRESS;
#if BITCOIN_TESTNET
				data[0] = ELA_STAND_ADDRESS;
#endif
				d = BRScriptData(elems[0], &l);
				BRHash160(&data[1], d, l);
				r = BRBase58CheckEncode(address.s, addrLen, data, 21);
			}
			else if (count == 2 && ((*elems[0] == OP_0 && (*elems[1] == 20 || *elems[1] == 32)) ||
			                        (*elems[0] >= OP_1 && *elems[0] <= OP_16 && *elems[1] >= 2 && *elems[1] <= 40))) {
				// pay-to-witness scriptPubKey
				r = BRBech32Encode(a, "bc", script);
#if BITCOIN_TESTNET
				r = BRBech32Encode(a, "tb", script);
#endif
				if (address.s && r > addrLen) r = 0;
				if (address.s) memcpy(address.s, a, r);
			}

			boost::shared_ptr<Address> addr = boost::shared_ptr<Address>(new Address(address));
			return addr;
		}

		boost::shared_ptr<Address> Address::fromScriptSignature(CMBlock script) {
			BRAddress address = {'\0'};
			size_t scriptLen = script.GetSize(), addrLen = sizeof(address.s);
			if (! script || scriptLen == 0 || scriptLen > MAX_SCRIPT_LENGTH) {
				throw std::logic_error("fromScriptSignature params error");
			}

			uint8_t data[21];
			const uint8_t *d = NULL, *elems[BRScriptElements(NULL, 0, script, scriptLen)];
			size_t l = 0, count = BRScriptElements(elems, sizeof(elems)/sizeof(*elems), script, scriptLen);

			data[0] = ELA_STAND_ADDRESS;
#if BITCOIN_TESTNET
			data[0] = ELA_STAND_ADDRESS;
#endif

			if (count >= 2 && *elems[count - 2] <= OP_PUSHDATA4 &&
			    (*elems[count - 1] == 65 || *elems[count - 1] == 33)) { // pay-to-pubkey-hash scriptSig
				d = BRScriptData(elems[count - 1], &l);
				if (l != 65 && l != 33) d = NULL;
				if (d) BRHash160(&data[1], d, l);
			}
			else if (count >= 2 && *elems[count - 2] <= OP_PUSHDATA4 && *elems[count - 1] <= OP_PUSHDATA4 &&
			         *elems[count - 1] > 0) { // pay-to-script-hash scriptSig
				data[0] = ELA_STAND_ADDRESS;
#if BITCOIN_TESTNET
				data[0] = ELA_STAND_ADDRESS;
#endif
				d = BRScriptData(elems[count - 1], &l);
				if (d) BRHash160(&data[1], d, l);
			}
			else if (count >= 1 && *elems[count - 1] <= OP_PUSHDATA4 && *elems[count - 1] > 0) { // pay-to-pubkey scriptSig
				// TODO: implement Peter Wullie's pubKey recovery from signature
			}
			// pay-to-witness scriptSig's are empty

			if (d) {
				BRBase58CheckEncode(address.s, addrLen, data, 21);
			}
			return boost::shared_ptr<Address>(new Address(address));
		}

		bool Address::isValid() {
			BRAddress* address = getRaw();
			bool res = Address::isValidAddress(address->s);
			return res;
		}

		CMBlock Address::getPubKeyScript() {
			BRAddress* address = getRaw();

			size_t pubKeyLen = BRAddressScriptPubKey(NULL, 0, address->s);
			CMBlock data(pubKeyLen);
			BRAddressScriptPubKey(data, pubKeyLen, address->s);

			return data;
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

	    bool Address::isValidAddress(const std::string &address) {
		    bool r = false;
		    if (address.size() <= 1) {
			    return r;
		    }
		    const char *addr = address.c_str();
		    uint8_t data[42];

		    if (BRBase58CheckDecode(data, sizeof(data), addr) == 21) {
			    r = (data[0] == ELA_STAND_ADDRESS || data[0] == ELA_CROSSCHAIN_ADDRESS ||
					    data[0] == ELA_MULTISIG_ADDRESS || data[0] == ELA_IDCHAIN_ADDRESS);
#if BITCOIN_TESTNET
			    r = (data[0] == ELA_STAND_ADDRESS || data[0] == ELA_CROSSCHAIN_ADDRESS ||
			    data[0] == ELA_MULTISIG_ADDRESS || data[0] == ELA_IDCHAIN_ADDRESS);
#endif
		    }

		    return r;
	    }

	    bool Address::UInt168IsValid(const UInt168 &u168) {
		    if (UInt168IsZero(&u168) == true) {
			    return false;
		    }
		    int prefix = u168.u8[0];
		    if (prefix != ELA_STAND_ADDRESS && prefix != ELA_MULTISIG_ADDRESS && prefix != ELA_CROSSCHAIN_ADDRESS &&
		        prefix != ELA_IDCHAIN_ADDRESS) {
			    return false;
		    }
		    return true;
	    }

	    bool Address::isValidIdAddress(const std::string &address) {
		    bool r = false;
		    if (address.size() <= 1) {
			    return r;
		    }
		    const char *addr = address.c_str();
		    uint8_t data[42];

		    if (BRBase58CheckDecode(data, sizeof(data), addr) == 21) {
			    r = data[0] == ELA_IDCHAIN_ADDRESS;
#if BITCOIN_TESTNET
			    r = data[0] == ELA_IDCHAIN_ADDRESS;
#endif
		    }

		    return r;
	    }
    }
}