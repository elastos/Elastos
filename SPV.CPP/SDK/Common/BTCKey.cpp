// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <openssl/ec.h>

#include "BTCKey.h"

namespace Elastos {
	namespace SDK {
		bool BTCKey::generateKey(CMemBlock<uint8_t> &privKey, CMemBlock<uint8_t> &pubKey, int nid) {
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

		CMemBlock<uint8_t> BTCKey::getPubKeyFromPrivKey(CMemBlock<uint8_t> privKey, int nid) {
			CMemBlock<uint8_t> out;
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

		bool BTCKey::PublickeyIsValid(CMemBlock<uint8_t> pubKey, int nid) {
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
	}
}
