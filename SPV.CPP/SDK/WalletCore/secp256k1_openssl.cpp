////////////////////////////////////////////////////////////////////////////////
//
// secp256k1_openssl.cpp
//
// Copyright (c) 2013-2014 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Some portions taken from bitcoin/bitcoin,
//      Copyright (c) 2009-2013 Satoshi Nakamoto, the Bitcoin developers
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#include "secp256k1_openssl.h"
#include <Common/hash.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>

#include <string>
#include <cassert>

namespace Elastos {
	namespace ElaWallet {

		bool static EC_KEY_regenerate_key(EC_KEY *eckey, BIGNUM *priv_key) {
			if (!eckey) return false;

			const EC_GROUP *group = EC_KEY_get0_group(eckey);

			bool rval = false;
			EC_POINT *pub_key = NULL;
			BN_CTX *ctx = BN_CTX_new();
			if (!ctx) goto finish;

			pub_key = EC_POINT_new(group);
			if (!pub_key) goto finish;

			if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx)) goto finish;

			EC_KEY_set_private_key(eckey, priv_key);
			EC_KEY_set_public_key(eckey, pub_key);

			rval = true;

finish:
			if (pub_key) EC_POINT_free(pub_key);
			if (ctx) BN_CTX_free(ctx);
			return rval;
		}

		void secp256k1_key::init() {
			_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			ErrorChecker::CheckLogic(!_key, Error::Key, "EC_KEY_new_by_curve_name failed");
			EC_KEY_set_conv_form(_key, POINT_CONVERSION_COMPRESSED);
		}

		secp256k1_key::secp256k1_key() :
			_key(nullptr) {

			}

		secp256k1_key &secp256k1_key::operator=(const secp256k1_key &from) {
			if (_key) EC_KEY_dup(from._key);
			_key = EC_KEY_dup(from._key);
			return *this;
		}

		EC_KEY *secp256k1_key::newKey() {
			if (!_key)
				init();

			if (!EC_KEY_generate_key(_key)) {
				Log::error("EC_KEY_generate_key failed");
				return nullptr;
			}

			EC_KEY_set_conv_form(_key, POINT_CONVERSION_COMPRESSED);
			return _key;
		}

		bytes_t secp256k1_key::getPrivKey() const {
			ErrorChecker::CheckLogic(!_key, Error::Key, "prv key is not set");

			const BIGNUM *bn = EC_KEY_get0_private_key(_key);
			ErrorChecker::CheckLogic(!bn, Error::Key, "get prv key fail");

			bytes_t privKey(32);
			assert(BN_num_bytes(bn) <= 32);
			BN_bn2bin(bn, &privKey[0]);
			return privKey;
		}

		EC_KEY *secp256k1_key::setPrivKey(const bytes_t &privkey) {
			if (!_key)
				init();

			BIGNUM *bn = BN_bin2bn(&privkey[0], privkey.size(), NULL);
			ErrorChecker::CheckLogic(!bn, Error::Key, "invalid prv key: 2bn fail");

			bool bFail = !EC_KEY_regenerate_key(_key, bn);
			BN_clear_free(bn);

			ErrorChecker::CheckLogic(bFail, Error::Key, "invalid prv key");

			ErrorChecker::CheckLogic(!EC_KEY_check_key(_key), Error::Key, "invalid prv key");

			return _key;
		}

		bytes_t secp256k1_key::getPubKey(bool bCompressed) const {
			ErrorChecker::CheckLogic(!_key, Error::Key, "key is not set");

			if (!bCompressed) EC_KEY_set_conv_form(_key, POINT_CONVERSION_UNCOMPRESSED);
			int nSize = i2o_ECPublicKey(_key, NULL);
			if (nSize == 0) {
				if (!bCompressed) EC_KEY_set_conv_form(_key, POINT_CONVERSION_COMPRESSED);
				ErrorChecker::ThrowLogicException(Error::Key, "i2o_ECPublicKey failed");
				return bytes_t();
			}

			bytes_t pubKey(nSize, 0);
			unsigned char *pBegin = &pubKey[0];
			if (i2o_ECPublicKey(_key, &pBegin) != nSize) {
				if (!bCompressed) EC_KEY_set_conv_form(_key, POINT_CONVERSION_COMPRESSED);
				ErrorChecker::ThrowLogicException(Error::Key, "i2o_ECPublicKey returned unexpected size");
				return bytes_t();
			}

			if (!bCompressed) EC_KEY_set_conv_form(_key, POINT_CONVERSION_COMPRESSED);
			return pubKey;
		}

		EC_KEY *secp256k1_key::setPubKey(const bytes_t &pubkey) {
			ErrorChecker::CheckLogic(pubkey.empty(), Error::Key, "pubkey is empty");
			if (!_key)
				init();

			const unsigned char *pBegin = (unsigned char *) &pubkey[0];
			if (!o2i_ECPublicKey(&_key, &pBegin, pubkey.size())) {
				ErrorChecker::ThrowLogicException(Error::Key, "o2i_ECPublicKey failed");
				return nullptr;
			}

			ErrorChecker::CheckLogic(!EC_KEY_check_key(_key), Error::Key, "invalid pub key");

			return _key;
		}



		secp256k1_point::secp256k1_point(const secp256k1_point &source) {
			init();
			if (!EC_GROUP_copy(group, source.group))
				ErrorChecker::ThrowLogicException(Error::Key,
						"EC_GROUP_copy failed.");
			if (!EC_POINT_copy(point, source.point))
				ErrorChecker::ThrowLogicException(Error::Key,
						"EC_POINT_copy failed.");
		}

		secp256k1_point::secp256k1_point(const bytes_t &bytes) {
			init();
			this->bytes(bytes);
		}


		secp256k1_point::~secp256k1_point() {
			if (point) EC_POINT_free(point);
			if (group) EC_GROUP_free(group);
			if (ctx) BN_CTX_free(ctx);
		}

		secp256k1_point &secp256k1_point::operator=(const secp256k1_point &rhs) {
			if (!EC_GROUP_copy(group, rhs.group))
				ErrorChecker::ThrowLogicException(Error::Key, "EC_GROUP_copy failed.");
			if (!EC_POINT_copy(point, rhs.point))
				ErrorChecker::ThrowLogicException(Error::Key, "EC_POINT_copy failed.");

			return *this;
		}

		BIGNUM *secp256k1_point::getCoordX() const {
			BIGNUM *x = BN_new();

			if (1 == EC_POINT_get_affine_coordinates_GFp(group, point, x, nullptr, nullptr)) {
				return x;
			}
			BN_free(x);

			return nullptr;
		}

		void secp256k1_point::bytes(const bytes_t &bytes) {
			std::string err;

			EC_POINT *rval;

			BIGNUM *bn = BN_bin2bn(&bytes[0], bytes.size(), NULL);
			if (!bn) {
				err = "BN_bin2bn failed.";
				goto finish;
			}

			rval = EC_POINT_bn2point(group, bn, point, ctx);
			if (!rval) {
				err = "EC_POINT_bn2point failed.";
				goto finish;
			}

finish:
			if (bn) BN_clear_free(bn);

			ErrorChecker::CheckLogic(!err.empty(), Error::Key, std::string("invalid key: ") + err);
		}

		bytes_t secp256k1_point::bytes() const {
			bytes_t bytes(33);

			std::string err;

			BIGNUM *rval;

			BIGNUM *bn = BN_new();
			if (!bn) {
				err = "BN_new failed.";
				goto finish;
			}

			rval = EC_POINT_point2bn(group, point, POINT_CONVERSION_COMPRESSED, bn, ctx);
			if (!rval) {
				err = "EC_POINT_point2bn failed.";
				goto finish;
			}

			assert(BN_num_bytes(bn) == 33);
			BN_bn2bin(bn, &bytes[0]);

finish:
			if (bn) BN_clear_free(bn);

			ErrorChecker::CheckLogic(!err.empty(), Error::Key, err);

			return bytes;
		}

		secp256k1_point &secp256k1_point::operator+=(const secp256k1_point &rhs) {
			if (!EC_POINT_add(group, point, point, rhs.point, ctx)) {
				ErrorChecker::ThrowLogicException(Error::Key, "EC_POINT_add failed.");
			}
			return *this;
		}

		secp256k1_point &secp256k1_point::operator*=(const bytes_t &rhs) {
			BIGNUM *bn = BN_bin2bn(&rhs[0], rhs.size(), NULL);
			ErrorChecker::CheckLogic(!bn, Error::Key, "BN_bin2bn failed.");

			int rval = EC_POINT_mul(group, point, NULL, point, bn, ctx);
			BN_clear_free(bn);

			ErrorChecker::CheckLogic(rval == 0, Error::Key, "EC_POINT_mul failed.");

			return *this;
		}

		// Computes n*G + K where K is this and G is the group generator
		void secp256k1_point::generator_mul(const bytes_t &n) {
			BIGNUM *bn = BN_bin2bn(&n[0], n.size(), NULL);
			ErrorChecker::CheckLogic(!bn, Error::Key, "BN_bin2bn failed.");

			//int rval = EC_POINT_mul(group, point, bn, (is_at_infinity() ? NULL : point), BN_value_one(), ctx);
			int rval = EC_POINT_mul(group, point, bn, point, BN_value_one(), ctx);
			BN_clear_free(bn);

			ErrorChecker::CheckLogic(rval == 0, Error::Key, "EC_POINT_mul failed.");
		}

		// Sets to n*G
		void secp256k1_point::set_generator_mul(const bytes_t &n) {
			BIGNUM *bn = BN_bin2bn(&n[0], n.size(), NULL);
			ErrorChecker::CheckLogic(!bn, Error::Key, "BN_bin2bn failed.");

			int rval = EC_POINT_mul(group, point, bn, NULL, NULL, ctx);
			BN_clear_free(bn);

			ErrorChecker::CheckLogic(rval == 0, Error::Key, "EC_POINT_mul failed.");
		}

		void secp256k1_point::init() {
			std::string err;

			group = NULL;
			point = NULL;
			ctx = NULL;

			group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
			if (!group) {
				err = "EC_KEY_new_by_curve_name failed.";
				goto finish;
			}

			point = EC_POINT_new(group);
			if (!point) {
				err = "EC_POINT_new failed.";
				goto finish;
			}

			ctx = BN_CTX_new();
			if (!ctx) {
				err = "BN_CTX_new failed.";
				goto finish;
			}

			return;

finish:
			if (group) EC_GROUP_free(group);
			if (point) EC_POINT_free(point);

			ErrorChecker::ThrowLogicException(Error::Key, err);
		}

	}
}

