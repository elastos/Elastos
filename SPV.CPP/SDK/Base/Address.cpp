// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>

#include <Core/BRBase58.h>
#include <Core/BRBech32.h>
#include <Core/BRAddress.h>

#define MAX_SCRIPT_LENGTH 0x100 // scripts over this size will not be parsed for an address

namespace Elastos {
	namespace ElaWallet {

		Address Address::None = Address();

		Address::Address() {
			memset(_s, 0, sizeof(_s));
		}

		Address::Address(const std::string &address) {
			operator=(address);
		}

		Address &Address::operator=(const std::string &address) {
			memset(_s, 0, sizeof(_s));
			strncpy(_s, address.c_str(), sizeof(_s) - 1);
			return *this;
		}

		bool Address::isValid() {
			bool res = Address::isValidAddress(_s);
			return res;
		}

		CMBlock Address::getPubKeyScript() {
			size_t pubKeyLen = BRAddressScriptPubKey(NULL, 0, _s);
			CMBlock data(pubKeyLen);
			BRAddressScriptPubKey(data, pubKeyLen, _s);

			return data;
		}

		int Address::getSignType() const {
			const char *addr = _s;
			uint8_t data[42];
			if (BRBase58CheckDecode(data, sizeof(data), addr) == 21) {
				if (data[0] == ELA_STAND_ADDRESS) {
					return ELA_STANDARD;
				} else if (data[0] == ELA_CROSSCHAIN_ADDRESS) {
					return ELA_CROSSCHAIN;
				} else if (data[0] == ELA_MULTISIG_ADDRESS) {
					return ELA_MULTISIG;
				} else if (data[0] == ELA_IDCHAIN_ADDRESS) {
					return ELA_IDCHAIN;
				} else if (data[0] == ELA_DESTROY_ADDRESS) {
					return ELA_DESTROY;
				} else {
					ParamChecker::checkCondition(true, Error::Address, "Unknown address type");
				}
		    } else {
				ParamChecker::checkCondition(true, Error::Address, "Invalid address");
		    }
		    return -1;
		}

		std::string Address::stringify() const {
			return _s;
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

		    if (r == 0 && strcmp(address.c_str(), ELA_SIDECHAIN_DESTROY_ADDR) == 0) {
		    	r = 1;
		    }

			return r;
		}

		bool Address::UInt168IsValid(const UInt168 &u168) {
			if (UInt168IsZero(&u168) == true) {
				return true;
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

		bool Address::isValidProgramHash(const UInt168 &u168, const Transaction::Type &type) {
			if (UInt168IsZero(&u168) == true) {
				return true;
			}

			int prefix = u168.u8[0];
			bool result = false;

			switch (type) {
				case Transaction::TransferCrossChainAsset:
					result = prefix == ELA_CROSSCHAIN_ADDRESS;
					break;
				case Transaction::TransferAsset:
					result = prefix == ELA_STAND_ADDRESS;
					break;
				case Transaction::RegisterIdentification:
					result = prefix == ELA_IDCHAIN_ADDRESS;
					break;
				default:
					result = Address::UInt168IsValid(u168);
					break;
			}
			return result;
		}

		bool Address::operator<(const Address &address) const {
			return strcmp(_s, address._s) < 0;
		}

		bool Address::IsEqual(const Address &otherAddr) const {
			return (this == &otherAddr || strncmp(_s, otherAddr._s, sizeof(_s)) == 0);
		}

		bool Address::operator==(const std::string &address) const {
			return IsEqual(address);
		}

		const char *Address::GetChar() const {
			return _s;
		}
	}
}
