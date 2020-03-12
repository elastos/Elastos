// Copyright (c) 2011 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "BigInt.h"
#include "ErrorChecker.h"

namespace Elastos {
	namespace ElaWallet {

		void BigInt::allocate() {
			this->autoclear = false;
			if (!(this->bn = BN_new())) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt allocate error");
			}
			if (!(this->ctx = BN_CTX_new())) {
				if (this->bn) BN_free(this->bn);
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt ctx new");
			}
		}

		BigInt::BigInt() {
			this->allocate();
			this->setUint64(0);
		}

		BigInt::BigInt(const BigInt &bigint) {
			this->autoclear = bigint.autoclear;

			if (!(this->ctx = BN_CTX_new())) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt ctx new");
			}

			if (!(this->bn = BN_dup(bigint.bn))) {
				if (this->ctx) BN_CTX_free(this->ctx);
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt dup");
			}
		}

		BigInt::BigInt(uint64_t num) {
			this->allocate();
			this->setUint64(num);
		}

		BigInt::BigInt(const std::vector<unsigned char> &bytes, bool bigEndian) {
			this->allocate();
			this->setBytes(bytes, bigEndian);
		}

		BigInt::BigInt(const std::string &inBase, unsigned int base, const char *alphabet) {
			this->allocate();
			this->setInBase(inBase, base, alphabet);
		}

		BigInt::~BigInt() {
			if (this->bn) {
				if (this->autoclear) BN_clear_free(this->bn);
				else BN_free(this->bn);
			}
			if (this->ctx) BN_CTX_free(this->ctx);
		}

		void BigInt::setAutoclear(bool autoclear) {
			this->autoclear = autoclear;
		}

		void BigInt::clear() {
			if (this->bn)
				BN_clear(this->bn);
		}

		BigInt &BigInt::operator=(const BigInt &bigint) {
			this->autoclear = bigint.autoclear;
			if (!(BN_copy(this->bn, bigint.bn)))
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt copy");
			//if (!(this->bn = BN_dup(bigint.bn))) throw std::runtime_error("BIGNUM allocation error.");
			//if (!(this->ctx = BN_CTX_new())) { BN_free(this->bn); throw std::runtime_error("BIGNUM allocation error."); }
			return *this;
		}

		BigInt &BigInt::operator=(uint64_t num) {
			this->setUint64(num);
			return *this;
		}

		BigInt &BigInt::operator+=(const BigInt &rhs) {
			if (!BN_add(this->bn, this->bn, rhs.bn)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt add");
			}

			return *this;
		}

		BigInt &BigInt::operator-=(const BigInt &rhs) {
			if (!BN_sub(this->bn, this->bn, rhs.bn)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt sub");
			}
			return *this;
		}

		BigInt &BigInt::operator*=(const BigInt &rhs) {
			if (!BN_mul(this->bn, this->bn, rhs.bn, this->ctx)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt mul");
			}
			return *this;
		}

		BigInt &BigInt::operator/=(const BigInt &rhs) {
			if (!BN_div(this->bn, NULL, this->bn, rhs.bn, this->ctx)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt /");
			}
			return *this;
		}

		BigInt &BigInt::operator%=(const BigInt &rhs) {
			if (!BN_div(NULL, this->bn, this->bn, rhs.bn, this->ctx)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt %");
			}
			return *this;
		}

		BigInt &BigInt::operator+=(uint64_t rhs) {
			BigInt rhsBG(rhs);
			if (!BN_add(this->bn, this->bn, rhsBG.bn))
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt +=");
			return *this;
		}

		BigInt &BigInt::operator-=(uint64_t rhs) {
			BigInt rhsBG(rhs);
			if (!BN_sub(this->bn, this->bn, rhsBG.bn)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt -=");
			}
			return *this;
		}

		BigInt &BigInt::operator*=(uint64_t rhs) {
			BigInt rhsBG(rhs);
			if (!BN_mul(this->bn, this->bn, rhsBG.bn, this->ctx)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt *=");
			}
			return *this;
		}

		BigInt &BigInt::operator/=(uint64_t rhs) {
			BigInt rhsBG(rhs);
			if (!BN_div(this->bn, NULL, this->bn, rhsBG.bn, this->ctx)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt /=");
			}
			return *this;
		}

		BigInt &BigInt::operator%=(uint64_t rhs) {
			BigInt rhsBG(rhs);
			if (!BN_div(NULL, this->bn, this->bn, rhsBG.bn, this->ctx)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt %=");
			}
			return *this;
		}

		BigInt BigInt::operator+(const BigInt &rightOperand) const {
			return BigInt(*this) += rightOperand;
		}

		BigInt BigInt::operator-(const BigInt &rightOperand) const {
			return BigInt(*this) -= rightOperand;
		}

		BigInt BigInt::operator*(const BigInt &rightOperand) const {
			return BigInt(*this) *= rightOperand;
		}

		BigInt BigInt::operator/(const BigInt &rightOperand) const {
			return BigInt(*this) /= rightOperand;
		}

		BigInt BigInt::operator%(const BigInt &rightOperand) const {
			return BigInt(*this) %= rightOperand;
		}

		BigInt BigInt::operator+(uint64_t rightOperand) const {
			return BigInt(*this) += rightOperand;
		}

		BigInt BigInt::operator-(uint64_t rightOperand) const {
			return BigInt(*this) -= rightOperand;
		}

		BigInt BigInt::operator*(uint64_t rightOperand) const {
			return BigInt(*this) *= rightOperand;
		}

		BigInt BigInt::operator/(uint64_t rightOperand) const {
			return BigInt(*this) /= rightOperand;
		}

		BigInt BigInt::operator%(uint64_t rightOperand) const {
			return BigInt(*this) %= rightOperand;
		}

		BigInt &BigInt::operator<<=(int rhs) {
			if (!BN_lshift(this->bn, this->bn, rhs)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt lshift");
			}
			return *this;
		}

		BigInt &BigInt::operator>>=(int rhs) {
			if (!BN_rshift(this->bn, this->bn, rhs)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt rshift");
			}
			return *this;
		}

		BigInt BigInt::operator<<(int rhs) const {
			return BigInt(*this) <<= rhs;
		}

		BigInt BigInt::operator>>(int rhs) const {
			return BigInt(*this) >>= rhs;
		}

		bool BigInt::operator==(const BigInt &rhs) const {
			return (BN_cmp(this->bn, rhs.bn) == 0);
		}

		bool BigInt::operator!=(const BigInt &rhs) const {
			return (BN_cmp(this->bn, rhs.bn) != 0);
		}

		bool BigInt::operator<(const BigInt &rhs) const {
			return (BN_cmp(this->bn, rhs.bn) < 0);
		}

		bool BigInt::operator>(const BigInt &rhs) const {
			return (BN_cmp(this->bn, rhs.bn) > 0);
		}

		bool BigInt::operator<=(const BigInt &rhs) const {
			return (BN_cmp(this->bn, rhs.bn) <= 0);
		}

		bool BigInt::operator>=(const BigInt &rhs) const {
			return (BN_cmp(this->bn, rhs.bn) >= 0);
		}

		bool BigInt::isZero() const {
			return BN_is_zero(this->bn);
		}

		uint64_t BigInt::getUint64() const {
			uint64_t num = 0;

			bytes_t bytes = getHexBytes(true);
			memcpy(&num, &bytes[0], MIN(sizeof(num), bytes.size()));

			return num;
		}

		void BigInt::setUint64(uint64_t num) {
			bytes_t bytes(&num, sizeof(num));
			setHexBytes(bytes, true);
		}

		// this method is not safe, will be deprecated
#if 0
		BN_ULONG BigInt::getWord() const {
			return BN_get_word(this->bn);
		}

		void BigInt::setWord(BN_ULONG num) {
			if (!BN_set_word(this->bn, num)) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt set word");
			}
		}
#endif

		void BigInt::setRaw(BIGNUM *bn) {
			if (this->bn) {
				if (this->autoclear) BN_clear_free(this->bn);
				else BN_free(this->bn);
			}
			this->bn = bn;
		}

		bytes_t BigInt::getHexBytes(bool littleEndian) const {
			bytes_t bytes;
			bytes.resize(BN_num_bytes(this->bn));

			char *hex = BN_bn2hex(this->bn);
			if (!hex) {
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt bn2hex");
			}

			bytes.setHex(std::string(hex));

			OPENSSL_free(hex);

			if (littleEndian)
				reverse(bytes.begin(), bytes.end());

			return bytes;
		}

		void BigInt::setHexBytes(bytes_t bytes, bool littleEndian) {
			if (littleEndian) reverse(bytes.begin(), bytes.end());
			BN_hex2bn(&this->bn, bytes.getHex().c_str());
		}

		bytes_t BigInt::getBytes(bool bigEndian) const {
			bytes_t bytes;
			bytes.resize(BN_num_bytes(this->bn));
			BN_bn2bin(this->bn, &bytes[0]);
			if (bigEndian) reverse(bytes.begin(), bytes.end());
			return bytes;
		}

		void BigInt::setBytes(bytes_t bytes, bool bigEndian) {
			if (bigEndian) reverse(bytes.begin(), bytes.end());
			BN_bin2bn(&bytes[0], bytes.size(), this->bn);
		}

		int BigInt::numBytes() const {
			return BN_num_bytes(this->bn);
		}

		std::string BigInt::getHex() const {
			char* hex = BN_bn2hex(this->bn);
			if (!hex)
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt bn2hex");
			std::string rval(hex);
			OPENSSL_free(hex);
			return rval;
		}
		void BigInt::setHex(const std::string& hex) {
			BN_hex2bn(&this->bn, hex.c_str());
		}

		void BigInt::SetHex(const std::string& hex) {
			setHex(hex);
		}

		std::string BigInt::getDec() const {
			char* dec = BN_bn2dec(this->bn);
			if (!dec)
				ErrorChecker::ThrowLogicException(Error::BigInt, "BigInt bn2dec");
			std::string rval(dec);
			OPENSSL_free(dec);
			return rval;
		}

		void BigInt::setDec(const std::string& dec) {
			BN_dec2bn(&this->bn, dec.c_str());
		}

		std::string BigInt::getInBase(unsigned int base, const char* alphabet) const {
			BigInt num = *this;
			std::string inBase;
			do {
				inBase = alphabet[(num % base).getUint64()] + inBase; // TODO: check whether this is most efficient structure manipulation
				num /= base;
			} while (!num.isZero());
			return inBase;
		}

		void BigInt::setInBase(const std::string& inBase, unsigned int base, const char* alphabet) {
			this->setUint64(0);
			for (unsigned int i = 0; i < inBase.size(); i++) {
				const char* pPos = strchr(alphabet, inBase[i]);
				if (!pPos) continue;
				*this *= base;
				*this += (pPos - alphabet);
			}
		}

	}
}