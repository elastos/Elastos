// Copyright (c) 2011 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include "BigInt.h"

namespace Elastos {
	namespace ElaWallet {

		void BigInt::allocate() {
			this->autoclear = false;
			if (!(this->bn = BN_new())) throw std::runtime_error("BIGNUM allocation error.");
			if (!(this->ctx = BN_CTX_new())) {
				BN_free(this->bn);
				throw std::runtime_error("BIGNUM allocation error.");
			}
		}

		BigInt::BigInt() {
			this->allocate();
			this->setWord(0);
		}

		BigInt::BigInt(const BigInt &bigint) {
			if (!(this->bn = BN_dup(bigint.bn)))
				throw std::runtime_error("BIGNUM allocation error.");

			if (!(this->ctx = BN_CTX_new())) {
				BN_free(this->bn);
				throw std::runtime_error("BIGNUM allocation error.");
			}
		}

		BigInt::BigInt(BN_ULONG num) {
			this->allocate();
			this->operator=(num);
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
			if (!(BN_copy(this->bn, bigint.bn))) throw std::runtime_error("BIGNUM allocation error.");
			//if (!(this->bn = BN_dup(bigint.bn))) throw std::runtime_error("BIGNUM allocation error.");
			//if (!(this->ctx = BN_CTX_new())) { BN_free(this->bn); throw std::runtime_error("BIGNUM allocation error."); }
			return *this;
		}

		BigInt &BigInt::operator=(BN_ULONG num) {
			this->setWord(num);
			return *this;
		}

		BigInt &BigInt::operator+=(const BigInt &rhs) {
			if (!BN_add(this->bn, this->bn, rhs.bn))
				throw std::runtime_error("BN_add error.");

			return *this;
		}

		BigInt &BigInt::operator-=(const BigInt &rhs) {
			if (!BN_sub(this->bn, this->bn, rhs.bn))
				throw std::runtime_error("BN_sub error.");
			return *this;
		}

		BigInt &BigInt::operator*=(const BigInt &rhs) {
			if (!BN_mul(this->bn, this->bn, rhs.bn, this->ctx))
				throw std::runtime_error("BN_mul rror.");
			return *this;
		}

		BigInt &BigInt::operator/=(const BigInt &rhs) {
			if (!BN_div(this->bn, NULL, this->bn, rhs.bn, this->ctx))
				throw std::runtime_error("BN_div error.");
			return *this;
		}

		BigInt &BigInt::operator%=(const BigInt &rhs) {
			if (!BN_div(NULL, this->bn, this->bn, rhs.bn, this->ctx))
				throw std::runtime_error("BN_div error.");
			return *this;
		}

		BigInt &BigInt::operator+=(BN_ULONG rhs) {
			if (!BN_add_word(this->bn, rhs))
				throw std::runtime_error("BN_add_word error.");
			return *this;
		}

		BigInt &BigInt::operator-=(BN_ULONG rhs) {
			if (!BN_sub_word(this->bn, rhs))
				throw std::runtime_error("BN_sub_word error.");
			return *this;
		}

		BigInt &BigInt::operator*=(BN_ULONG rhs) {
			if (!BN_mul_word(this->bn, rhs))
				throw std::runtime_error("BN_mul_word error.");
			return *this;
		}

		BigInt &BigInt::operator/=(BN_ULONG rhs) {
			BN_div_word(this->bn, rhs);
			return *this;
		}

		BigInt &BigInt::operator%=(BN_ULONG rhs) {
			this->setWord(BN_mod_word(this->bn, rhs));
			return *this;
		}

		const BigInt BigInt::operator+(const BigInt &rightOperand) const {
			return BigInt(*this) += rightOperand;
		}

		const BigInt BigInt::operator-(const BigInt &rightOperand) const {
			return BigInt(*this) -= rightOperand;
		}

		const BigInt BigInt::operator*(const BigInt &rightOperand) const {
			return BigInt(*this) *= rightOperand;
		}

		const BigInt BigInt::operator/(const BigInt &rightOperand) const {
			return BigInt(*this) /= rightOperand;
		}

		const BigInt BigInt::operator%(const BigInt &rightOperand) const {
			return BigInt(*this) %= rightOperand;
		}

		const BigInt BigInt::operator+(BN_ULONG rightOperand) const {
			return BigInt(*this) += rightOperand;
		}

		const BigInt BigInt::operator-(BN_ULONG rightOperand) const {
			return BigInt(*this) -= rightOperand;
		}

		const BigInt BigInt::operator*(BN_ULONG rightOperand) const {
			return BigInt(*this) *= rightOperand;
		}

		const BigInt BigInt::operator/(BN_ULONG rightOperand) const {
			return BigInt(*this) /= rightOperand;
		}

		BN_ULONG BigInt::operator%(BN_ULONG rightOperand) const {
			return BN_mod_word(this->bn, rightOperand);
		}

		BigInt &BigInt::operator<<=(int rhs) {
			if (!BN_lshift(this->bn, this->bn, rhs))
				throw std::runtime_error("BN_lshift error.");
			return *this;
		}

		BigInt &BigInt::operator>>=(int rhs) {
			if (!BN_rshift(this->bn, this->bn, rhs))
				throw std::runtime_error("BN_rshift error.");
			return *this;
		}

		const BigInt BigInt::operator<<(int rhs) const {
			return BigInt(*this) <<= rhs;
		}

		const BigInt BigInt::operator>>(int rhs) const {
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

		BN_ULONG BigInt::getWord() const {
			return BN_get_word(this->bn);
		}

		void BigInt::setWord(BN_ULONG num) {
			if (!BN_set_word(this->bn, num))
				throw std::runtime_error("BN_set_word error.");
		}

		void BigInt::setRaw(BIGNUM *bn) {
			if (this->bn) {
				if (this->autoclear) BN_clear_free(this->bn);
				else BN_free(this->bn);
			}
			this->bn = bn;
		}

		bytes_t BigInt::getHexBytes(bool littleEndian ) const {
			bytes_t bytes;
			bytes.resize(BN_num_bytes(this->bn));

			char *hex = BN_bn2hex(this->bn);
			if (!hex)
				throw std::runtime_error("BN_bn2hex error.");

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
			if (!hex) throw std::runtime_error("BN_bn2hex error.");
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
			if (!dec) throw std::runtime_error("BN_bn2dec error.");
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
				inBase = alphabet[num % base] + inBase; // TODO: check whether this is most efficient structure manipulation
				num /= base;
			} while (!num.isZero());
			return inBase;
		}

		void BigInt::setInBase(const std::string& inBase, unsigned int base, const char* alphabet) {
			this->setWord(0);
			for (unsigned int i = 0; i < inBase.size(); i++) {
				const char* pPos = strchr(alphabet, inBase[i]);
				if (!pPos) continue;
				*this *= base;
				*this += (pPos - alphabet);
			}
		}

	}
}