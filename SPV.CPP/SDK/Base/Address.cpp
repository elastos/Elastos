// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Base58.h>

#include <Core/BRBech32.h>

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
			return Address::isValidAddress(_s);
		}

		std::string Address::stringify() const {
			return _s;
		}

		bool Address::isValidAddress(const std::string &addr) {
			bool valid = false;

			CMBlock programHash = Base58::CheckDecode(addr);
			if (programHash.GetSize() == 21) {
				valid = programHash[0] == PrefixStandard ||
					programHash[0] == PrefixCrossChain ||
					programHash[0] == PrefixMultiSign ||
					programHash[0] == PrefixIDChain ||
					programHash[0] == PrefixDeposit;
			}

			if (!valid && addr == ELA_SIDECHAIN_DESTROY_ADDR) {
				valid = true;
			}

			return valid;
		}

		bool Address::UInt168IsValid(const UInt168 &u) {
			if (UInt168IsZero(&u)) {
				return true;
			}

			return u.u8[0] == PrefixStandard ||
				   u.u8[0] == PrefixCrossChain ||
				   u.u8[0] == PrefixMultiSign ||
				   u.u8[0] == PrefixIDChain ||
				   u.u8[0] == PrefixDeposit;
		}

		bool Address::isValidIdAddress(const std::string &addr) {
			bool valid = false;

			CMBlock programHash = Base58::CheckDecode(addr);
			if (programHash.GetSize() == 21) {
				valid = programHash[0] == PrefixIDChain;
			}

			return valid;
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
