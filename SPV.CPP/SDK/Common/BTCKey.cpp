// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <openssl/ec.h>

#include "BTCKey.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {
		bool BTCKey::generateKey(CMBlock &privKey, CMBlock &pubKey, int nid) {
			bool out = false;
			EC_GROUP *curve = nullptr;
			if (nullptr != (curve = EC_GROUP_new_by_curve_name(nid))) {
				EC_KEY *key = EC_KEY_new();
				if (nullptr != key) {
					if (1 == EC_KEY_set_group(key, curve)) {
						if (1 == EC_KEY_generate_key(key)) {
							const BIGNUM *_privkey = nullptr;
							const EC_POINT *_pubkey = nullptr;
							if (nullptr != (_privkey = EC_KEY_get0_private_key(key)) &&
								nullptr != (_pubkey = EC_KEY_get0_public_key(key))) {
								uint8_t arrBN[256] = {0};
								int len = BN_bn2bin(_privkey, arrBN);
								if (0 < len) {
									privKey.Resize((size_t) len);
									memcpy(privKey, arrBN, (size_t) len);
								}
								BIGNUM *__pubkey = EC_POINT_point2bn(curve, _pubkey, POINT_CONVERSION_COMPRESSED,
																	 nullptr, nullptr);
								if (nullptr != __pubkey) {
									len = BN_bn2bin(__pubkey, arrBN);
									if (0 < len) {
										pubKey.Resize(len);
										memcpy(pubKey, arrBN, (size_t) len);
										out = true;
									}
									BN_free(__pubkey);
								}
							}
						}
					}
					EC_KEY_free(key);
				}
				EC_GROUP_free(curve);
			}
			return out;
		}

		CMBlock BTCKey::getPubKeyFromPrivKey(CMBlock privKey, int nid) {
			CMBlock out;
			BIGNUM *_privkey = nullptr;
			if (32 != privKey.GetSize()) {
				return out;
			} else {
				_privkey = BN_bin2bn((const unsigned char *) (uint8_t *) privKey, (int) privKey.GetSize(), nullptr);
			}
			EC_KEY *key = EC_KEY_new_by_curve_name(nid);
			if (nullptr != _privkey && nullptr != key) {
				const EC_GROUP *curve = EC_KEY_get0_group(key);
				EC_POINT *_pubkey = EC_POINT_new(curve);
				if (_pubkey) {
					if (1 == EC_POINT_mul(curve, _pubkey, _privkey, NULL, NULL, NULL)) {
						BIGNUM *__pubkey = EC_POINT_point2bn(curve, _pubkey, POINT_CONVERSION_COMPRESSED, nullptr,
															 nullptr);
						if (nullptr != __pubkey) {
							uint8_t arrBN[256] = {0};
							int len = BN_bn2bin(__pubkey, arrBN);
							if (0 < len) {
								out.Resize(len);
								memcpy(out, arrBN, (size_t) len);
							}
							BN_free(__pubkey);
						}
					}
					EC_POINT_free(_pubkey);
				}
				BN_free(_privkey);
				EC_KEY_free(key);
			}
			return out;
		}

		bool BTCKey::PublickeyIsValid(CMBlock pubKey, int nid) {
			bool out = false;
			if (0 == pubKey.GetSize()) {
				return out;
			}
			EC_KEY *key = EC_KEY_new_by_curve_name(nid);
			if (nullptr != key) {
				BIGNUM *_pubkey = BN_bin2bn((const unsigned char *) (uint8_t *) pubKey, (int) pubKey.GetSize(),
											nullptr);
				if (nullptr != _pubkey) {
					const EC_GROUP *curve = EC_KEY_get0_group(key);
					EC_POINT *ec_p = EC_POINT_bn2point(curve, _pubkey, nullptr, nullptr);
					if (nullptr != ec_p) {
						if (1 == EC_KEY_set_public_key(key, ec_p)) {
							if (1 == EC_KEY_check_key(key)) {
								out = true;
							}
						}
						EC_POINT_free(ec_p);
					}
					BN_free(_pubkey);
				}
				EC_KEY_free(key);
			}
			return out;
		}

		CMBlock BTCKey::SignCompact(const CMBlock &privKey, const CMBlock msg) {
			const int nid = NID_X9_62_prime256v1;
			EC_KEY *ec_key = EC_KEY_new_by_curve_name(nid);
			if (ec_key == nullptr) {
				throw std::logic_error("EC_KEY_new_by_curve_name error by nid");
			}

			if (privKey.GetSize() != 32) {
				throw std::logic_error("error private key data ");
			}

			BIGNUM *privkey = BN_bin2bn((const uint8_t *) (uint8_t *) privKey, (int) privKey.GetSize(), nullptr);
			EC_KEY_generate_key(ec_key);
			EC_KEY_set_private_key(ec_key, privkey);

			ECDSA_SIG *sig = ECDSA_do_sign(msg, 32, ec_key);
			if (sig == nullptr) {
				throw std::logic_error("ECDSA_do_sign error!");
			}
			const BIGNUM *r = nullptr;
			const BIGNUM *s = nullptr;
			ECDSA_SIG_get0(sig, &r, &s);
			int nBitsR = BN_num_bits(r);
			int nBitsS = BN_num_bits(s);
			bool fOk = false;
			int rec = -1;
			if (nBitsR <= 256 && nBitsS <= 256) {
				CMBlock pubKey = getPubKeyFromPrivKey(privKey, nid);
				for (int i = 0; i < 4; ++i) {
					if (ECDSA_SIG_recover_key_GFp(ec_key, sig, msg, msg.GetSize(), i , 1) == 1) {
						EC_KEY_set_conv_form(ec_key, POINT_CONVERSION_COMPRESSED);
						int nSize = i2o_ECPublicKey(ec_key, NULL);
						assert(nSize);
						assert(nSize <= 65);
						unsigned char c[65];
						memset(c, 0, 65);
						unsigned char *pbegin = c;
						nSize = i2o_ECPublicKey(ec_key, &pbegin);
						CMBlock tempPubKey;
						tempPubKey.SetMemFixed(&c[0], nSize);
						if (memcmp(tempPubKey, pubKey, nSize) == 0) {
							rec = i;
							fOk = true;
							break;
						}
					}
				}
			}

			assert(fOk);

			CMBlock sigData(64);
			BN_bn2bin(r, &sigData[32 - (nBitsR + 7) / 8]);
			BN_bn2bin(s, &sigData[64 - (nBitsS + 7) / 8]);

			CMBlock signature(65);
			signature[0] = 27 + rec + 4;
			memcpy(&signature[1], sigData, sigData.GetSize());
			BN_free(privkey);
			ECDSA_SIG_free(sig);
			EC_KEY_free(ec_key);
			return signature;
		}

		bool BTCKey::VerifyCompact(const std::string &publicKey, const UInt256 &msg, const CMBlock &signature) {
			const int nid = NID_X9_62_prime256v1;
			CMBlock pubData = Utils::decodeHex(publicKey);
			CMBlock publicData;
			publicData.SetMemFixed(pubData, pubData.GetSize());
			if (!BTCKey::PublickeyIsValid(publicData, nid)) {
				return false;
			}
			if(signature.GetSize() != 65) {
				return false;
			}
			int recid = (signature[0] - 27) & 3;
			if (recid < 0 || recid >= 3)
				return false;

			EC_KEY *ec_key = EC_KEY_new_by_curve_name(nid);
			BIGNUM *pubKey = BN_bin2bn((const unsigned char *) (uint8_t *) publicData, (int) publicData.GetSize(),
			                           nullptr);

			const EC_GROUP *curve = EC_KEY_get0_group(ec_key);
			EC_POINT *ec_p = EC_POINT_bn2point(curve, pubKey, nullptr, nullptr);
			EC_KEY_set_public_key(ec_key, ec_p);

			const uint8_t *p64 = (const uint8_t *)&signature[1];
			ECDSA_SIG *sig = ECDSA_SIG_new();

			BIGNUM *r = BN_bin2bn(&p64[0], 32, nullptr);
			BIGNUM *s = BN_bin2bn(&p64[32], 32, nullptr);
			ECDSA_SIG_set0(sig, r, s);

			bool ret = ECDSA_SIG_recover_key_GFp(ec_key, sig, (unsigned char*)&msg, sizeof(msg), recid, 0) == 1;
			if (ret) {
				EC_KEY_set_conv_form(ec_key, POINT_CONVERSION_COMPRESSED);
				int nSize = i2o_ECPublicKey(ec_key, NULL);
				assert(nSize);
				assert(nSize <= 65);
				unsigned char c[65];
				memset(c, 0, 65);
				unsigned char *pbegin = c;
				nSize = i2o_ECPublicKey(ec_key, &pbegin);
				CMBlock tempPubKey;
				tempPubKey.SetMemFixed(&c[0], nSize);
				ret = memcmp(tempPubKey, publicData, nSize) == 0;
			}

			BN_free(pubKey);
			EC_POINT_free(ec_p);
			ECDSA_SIG_free(sig);
			EC_KEY_free(ec_key);

			return ret;
		}

		size_t BTCKey::ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const uint8_t *msg, size_t msglen,
		                                         size_t recid, size_t check) {
			if (!eckey) {
				return 0;
			}
			BN_CTX *ctx = nullptr;
			BIGNUM *x = nullptr, *e = nullptr, *order = nullptr, *sor = nullptr, *eor = nullptr, *field = nullptr;
			EC_POINT *R = nullptr, *O = nullptr, *Q = nullptr;
			BIGNUM *rr = nullptr, *zero = nullptr;
			size_t n = 0, ret = 0;
			size_t i = recid / 2;
			const BIGNUM *r = nullptr;
			const BIGNUM *s = nullptr;
			ECDSA_SIG_get0(ecsig, &r, &s);
			const EC_GROUP *group = EC_KEY_get0_group(eckey);
			ctx = BN_CTX_new();
			if (!ctx) {
				ret = -1;
				goto  err;
			}
			BN_CTX_start(ctx);
			order = BN_CTX_get(ctx);
			if (!EC_GROUP_get_order(group, order, ctx)) {
				ret = -2;
				goto err;
			}
			x = BN_CTX_get(ctx);
			if (!BN_copy(x, order)) {
				ret = -1;
				goto err;
			}
			if (!BN_mul_word(x, i)) {
				ret = -1;
				goto err;
			}

			if (!BN_add(x, x, r)) {
				ret = -1;
				goto err;
			}
			field = BN_CTX_get(ctx);
			if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) {
				ret = -2;
				goto err;
			}
			if (BN_cmp(x, field) >= 0) {
				ret = 0;
				goto err;
			}
			if ((R = EC_POINT_new(group)) == NULL) {
				ret = -2;
				goto err;
			}
			if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) {
				ret = 0;
				goto err;
			}
			if (check)
			{
				if ((O = EC_POINT_new(group)) == NULL) {
					ret = -2;
					goto err;
				}
				if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) {
					ret = -2;
					goto err;
				}
				if (!EC_POINT_is_at_infinity(group, O)) {
					ret = 0;
					goto err;
				}
			}
			if ((Q = EC_POINT_new(group)) == NULL) {
				ret = -2;
				goto err;
			}
			n = EC_GROUP_get_degree(group);
			e = BN_CTX_get(ctx);
			if (!BN_bin2bn(msg, msglen, e)) {
				ret = -1;
				goto err;
			}
			if (8*msglen > n) BN_rshift(e, e, 8-(n & 7));
			zero = BN_CTX_get(ctx);
			if (!BN_zero(zero)) {
				ret = -1;
				goto err;
			}
			if (!BN_mod_sub(e, zero, e, order, ctx)) {
				ret = -1;
				goto err;
			}
			rr = BN_CTX_get(ctx);
			if (!BN_mod_inverse(rr, r, order, ctx)) {
				ret = -1;
				goto err;
			}
			sor = BN_CTX_get(ctx);
			if (!BN_mod_mul(sor, s, rr, order, ctx)) {
				ret = -1;
				goto err;
			}
			eor = BN_CTX_get(ctx);
			if (!BN_mod_mul(eor, e, rr, order, ctx)) {
				ret = -1;
				goto err;
			}
			if (!EC_POINT_mul(group, Q, eor, R, sor, ctx)) {
				ret = -2;
				goto err;
			}
			if (!EC_KEY_set_public_key(eckey, Q)) {
				ret = -2;
				goto err;
			}

			ret = 1;

			err:
			if (ctx) {
				BN_CTX_end(ctx);
				BN_CTX_free(ctx);
			}
			if (R != NULL) EC_POINT_free(R);
			if (O != NULL) EC_POINT_free(O);
			if (Q != NULL) EC_POINT_free(Q);

			return ret;
		}
	}
}
