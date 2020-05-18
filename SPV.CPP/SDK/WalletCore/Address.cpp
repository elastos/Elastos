// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Address.h"

#include <WalletCore/Base58.h>
#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/ErrorChecker.h>
#include <Common/hash.h>
#include <Common/BigInt.h>
#include <WalletCore/secp256k1_openssl.h>

#include <boost/bind.hpp>

namespace Elastos {
	namespace ElaWallet {

		Address::Address() {
			_isValid = false;
		}

		Address::Address(const std::string &address) {
			_str = address;
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

		Address::Address(Prefix prefix, const bytes_t &pubKey, bool did) :
			Address(prefix, {pubKey}, 1, did) {
		}

		Address::Address(Prefix prefix, const std::vector<bytes_t> &pubkeys, uint8_t m, bool did) {
			if (pubkeys.size() == 0) {
				_isValid = false;
			} else {
				GenerateCode(prefix, pubkeys, m, did);
				GenerateProgramHash(prefix);
				if (CheckValid())
					_str = Base58::CheckEncode(_programHash.bytes());
			}
		}

		Address::Address(const uint168 &programHash) {
			_programHash = programHash;
			if (CheckValid())
				_str = Base58::CheckEncode(_programHash.bytes());
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
			return _isValid && _programHash.prefix() == PrefixIDChain;
		}

		std::string Address::String() const {
			return _str;
		}

		const uint168 &Address::ProgramHash() const {
			return _programHash;
		}

		void Address::SetProgramHash(const uint168 &programHash) {
			_programHash = programHash;
			if (CheckValid())
				_str = Base58::CheckEncode(_programHash.bytes());
		}

		SignType Address::PrefixToSignType(Prefix prefix) const {
			SignType type;

			switch (prefix) {
				case PrefixIDChain:
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

		void Address::SetRedeemScript(Prefix prefix, const bytes_t &code) {
			_code = code;
			GenerateProgramHash(prefix);
			if (CheckValid())
				_str = Base58::CheckEncode(_programHash.bytes());
			ErrorChecker::CheckCondition(!_isValid, Error::InvalidArgument, "redeemscript is invalid");
		}

		bool Address::ChangePrefix(Prefix prefix) {
			ErrorChecker::CheckCondition(!_isValid, Error::Address, "can't change prefix with invalid addr");
			SignType oldSignType = SignType(_code.back());
			if (oldSignType == SignTypeMultiSign || PrefixToSignType(prefix) == SignTypeMultiSign)
				ErrorChecker::ThrowLogicException(Error::Address, "can't change to or from multi-sign prefix");

			GenerateProgramHash(prefix);
			_str = Base58::CheckEncode(_programHash.bytes());
			return true;
		}

		void Address::ConvertToDID() {
			if (!_code.empty() && _programHash.prefix() == PrefixIDChain) {
				_code.back() = SignTypeDID;
				GenerateProgramHash(PrefixIDChain);
				_str = Base58::CheckEncode(_programHash.bytes());
			}
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
			_str = address._str;
			return *this;
		}

		bool Address::operator==(const Address &address) const {
			return _isValid == address._isValid && _programHash == address._programHash;
		}

		bool Address::operator==(const std::string &address) const {
			return _isValid && this->String() == address;
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

		void Address::GenerateCode(Prefix prefix, const std::vector<bytes_t> &pubkeys, uint8_t m, bool did) {
			ErrorChecker::CheckLogic(m > pubkeys.size() || m == 0, Error::MultiSignersCount, "Invalid m");

			if (m == 1 && pubkeys.size() == 1) {
				_code.push_back(pubkeys[0].size());
				_code += pubkeys[0];
				if (did)
					_code.push_back(SignTypeDID);
				else
					_code.push_back(PrefixToSignType(prefix));
			} else {
				ErrorChecker::CheckCondition(pubkeys.size() > sizeof(uint8_t) - OP_1, Error::MultiSignersCount,
											 "Signers should less than 205.");

				std::vector<bytes_t> sortedSigners(pubkeys.begin(), pubkeys.end());
				std::sort(sortedSigners.begin(), sortedSigners.end(), [](const bytes_t &a, const bytes_t &b) {
					return a.getHex() < b.getHex();
				});

				_code.push_back(uint8_t(OP_1 + m - 1));
				for (size_t i = 0; i < sortedSigners.size(); i++) {
					_code.push_back(uint8_t(sortedSigners[i].size()));
					_code += sortedSigners[i];
				}
				_code.push_back(uint8_t(OP_1 + sortedSigners.size() - 1));
				_code.push_back(PrefixToSignType(prefix));
			}
		}

		void Address::GenerateProgramHash(Prefix prefix) {
			bytes_t hash = hash160(_code);
			_programHash = uint168(prefix, hash);
		}

		bool Address::CheckValid() {
			if (_programHash.prefix() == PrefixDeposit ||
				_programHash.prefix() == PrefixStandard ||
				_programHash.prefix() == PrefixCrossChain ||
				_programHash.prefix() == PrefixMultiSign ||
				_programHash.prefix() == PrefixIDChain ||
				_programHash.prefix() == PrefixDestroy ||
				_programHash.prefix() == PrefixCRExpenses) {
				_isValid = true;
			} else {
				_isValid = false;
			}

			return _isValid;
		}

	}
}