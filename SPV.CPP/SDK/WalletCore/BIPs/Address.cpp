// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"

#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/hash.h>
#include <SDK/Common/BigInt.h>
#include <SDK/WalletCore/BIPs/secp256k1_openssl.h>

#include <boost/bind.hpp>

namespace Elastos {
	namespace ElaWallet {

		Address::Address() {
			_isValid = false;
		}

		Address::Address(const std::string &address) {
			if (address.empty()) {
				_isValid = false;
			} else {
				bytes_t payload;
				if (Base58::CheckDecode(address, payload)) {
					_programHash = uint168(payload);
					CheckValid();
				} else {
					Log::error("invalid address {}", address);
					_isValid = false;
				}
			}
		}

		Address::Address(Prefix prefix, const bytes_t &pubKey) :
			Address(prefix, {pubKey}, 1) {
		}

		Address::Address(Prefix prefix, const std::vector<bytes_t> &pubkeys, uint8_t m) {
			if (pubkeys.size() == 0) {
				_isValid = false;
			} else {
				GenerateCode(prefix, pubkeys, m);
				GenerateProgramHash(prefix);
				CheckValid();
			}
		}

		Address::Address(const uint168 &programHash) {
			_programHash = programHash;
			CheckValid();
		}

		Address::Address(const Address &address) {
			operator=(address);
		}

		Address::~Address() {

		}

		bool Address::Valid() const {
			return _isValid;
		}

		bool Address::IsIDAddress() const {
			if (_isValid && _programHash.prefix() == PrefixIDChain)
				return true;

			return false;
		}

		std::string Address::String() const {
			if (!_isValid)
				return std::string();

			return Base58::CheckEncode(_programHash.bytes());
		}

		const uint168 &Address::ProgramHash() const {
			return _programHash;
		}

		SignType Address::PrefixToSignType(Prefix prefix) const {
			SignType type;

			switch (prefix) {
				case PrefixStandard:
				case PrefixDeposit:
					type = SignTypeStandard;
					break;
				case PrefixCrossChain:
					type = SignTypeCrossChain;
					break;
				case PrefixMultiSign:
					type = SignTypeMultiSign;
					break;
				case PrefixIDChain:
					type = SignTypeIDChain;
					break;
				case PrefixDestroy:
					type = SignTypeDestroy;
					break;
				default:
					Log::error("invalid prefix {}", prefix);
					type = SignTypeInvalid;
					break;
			}

			return type;
		}

		const bytes_t &Address::RedeemScript() const {
			assert(!_code.empty());
			return _code;
		}

		bool Address::operator<(const Address &address) const {
			return _programHash < address._programHash;
		}

		Address& Address::operator=(const Address &address) {
			_programHash = address._programHash;
			_code = address._code;
			_isValid = address._isValid;
			return *this;
		}

		bool Address::operator==(const Address &address) const {
			return this == &address || _programHash == address._programHash;
		}

		bool Address::operator==(const std::string &address) const {
			return this->String() == address;
		}

		bool Address::operator!=(const Address &address) const {
			return _programHash != address._programHash;
		}

		bool Address::operator!=(const std::string &address) const {
			return this->String() != address;
		}

		bool Address::Compare(const bytes_t &a, const bytes_t &b) const {
			secp256k1_point pnt;
			BIGNUM *bn;

			pnt.bytes(a);
			bn = pnt.getCoordX();
			assert(bn != nullptr);
			BigInt bigIntA;
			bigIntA.setRaw(bn);

			pnt.bytes(b);
			bn = pnt.getCoordX();
			assert(bn != nullptr);
			BigInt bigIntB;
			bigIntB.setRaw(bn);

			return bigIntA <= bigIntB;
		}

		void Address::GenerateCode(Prefix prefix, const std::vector<bytes_t> &pubkeys, uint8_t m) {
			ErrorChecker::CheckLogic(m > pubkeys.size() || m == 0, Error::MultiSignersCount, "Invalid m");

			if (m == 1 && pubkeys.size() == 1) {
				_code.push_back(pubkeys[0].size());
				_code += pubkeys[0];
				_code.push_back(PrefixToSignType(prefix));
			} else {
				ErrorChecker::CheckCondition(pubkeys.size() > sizeof(uint8_t) - OP_1, Error::MultiSignersCount,
											 "Signers should less than 205.");

				std::vector<bytes_t> sortedSigners(pubkeys.begin(), pubkeys.end());
				std::sort(sortedSigners.begin(), sortedSigners.end(),
						  boost::bind(&Address::Compare, this, _1, _2));

				_code.push_back(uint8_t(OP_1 + m - 1));
				for (size_t i = 0; i < sortedSigners.size(); i++) {
					_code.push_back(uint8_t(sortedSigners[i].size()));
					_code += sortedSigners[i];
				}
				_code.push_back(uint8_t(OP_1 + sortedSigners.size() - 1));
				_code.push_back(SignTypeMultiSign);
			}
		}

		void Address::GenerateProgramHash(Prefix prefix) {
			bytes_t hash = hash160(_code);
			_programHash = uint168(prefix, hash);
		}

		void Address::CheckValid() {
			if (_programHash.prefix() == PrefixDeposit ||
				_programHash.prefix() == PrefixStandard ||
				_programHash.prefix() == PrefixCrossChain ||
				_programHash.prefix() == PrefixMultiSign ||
				_programHash.prefix() == PrefixIDChain ||
				_programHash.prefix() == PrefixDestroy) {
				_isValid = true;
			} else {
				_isValid = false;
			}
		}

	}
}