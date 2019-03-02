// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"

#include <SDK/Common/Base58.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Crypto/Key.h>

namespace Elastos {
	namespace ElaWallet {

		Address::Address() {
			memset(_programHash.u8, 0, sizeof(_programHash));
			_isValid = false;
		}

		Address::Address(const std::string &address) {

			if (address.empty()) {
				memset(_programHash.u8, 0, sizeof(_programHash));
				_isValid = false;
			} else {
				CMBlock programHash = Base58::CheckDecode(address);

				if (programHash.GetSize() != sizeof(UInt168)) {
					Log::error("invalid address {}", address);
					memset(_programHash.u8, 0, sizeof(_programHash));
					_isValid = false;
				} else {
					_isValid = true;
					memcpy(_programHash.u8, programHash, programHash.GetSize());
				}
			}
		}

		Address::Address(const CMBlock &pubKey, Prefix prefix) {
			Key key;
			if (!key.SetPubKey(pubKey)) {
				Log::error("invalid pubKey {}", Utils::encodeHex(pubKey));
				memset(_programHash.u8, 0, sizeof(_programHash));
				_isValid = false;
			} else {
				_programHash = key.ProgramHash(prefix);
				_isValid = true;
			}
		}

		Address::Address(const UInt168 &programHash) {
			_programHash = programHash;
			_isValid = true;
		}

		Address::Address(const Address &address) {
			operator=(address);
		}

		Address::~Address() {

		}

		bool Address::IsValid() const {
			return _isValid;
		}

		std::string Address::String() const {
			if (!_isValid)
				return std::string();

			return Base58::CheckEncode(_programHash.u8, sizeof(_programHash));
		}

		const UInt168 &Address::ProgramHash() const {
			return _programHash;
		}

		bool Address::operator<(const Address &address) const {
			return memcmp(_programHash.u8, address._programHash.u8, sizeof(_programHash)) < 0;
		}

		Address& Address::operator=(const Address &address) {
			_programHash = address._programHash;
			_isValid = address._isValid;
			return *this;
		}

		bool Address::operator==(const Address &address) const {
			return this == &address || memcmp(_programHash.u8, address._programHash.u8, sizeof(_programHash)) == 0;
		}

		bool Address::operator==(const std::string &address) const {
			return this->String() == address;
		}

		bool Address::operator!=(const Address &address) const {
			return memcmp(_programHash.u8, address._programHash.u8, sizeof(_programHash)) != 0;
		}

		bool Address::operator!=(const std::string &address) const {
			return this->String() != address;
		}

	}
}