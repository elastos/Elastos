// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//Add * accorded with breadwallet by zhangcl 791398105@qq.com

#include <fstream>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/objects.h>
#include <Core/BRKey.h>

#include "BRBIP39WordsEn.h"
#include "BRCrypto.h"
#include "BRKey.h"
#include "BTCKey.h"
#include "BTCBase58.h"
#include "WalletTool.h"

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
								unsigned char arrBN[256] = {0};
								int len = BN_bn2bin(_privkey, arrBN);
								if (0 < len) {
									privKey.Resize((size_t) len);
									memcpy((void *) privKey, arrBN, (size_t) len);
								}
								BIGNUM *__pubkey = EC_POINT_point2bn(curve, _pubkey, POINT_CONVERSION_COMPRESSED,
																	 nullptr, nullptr);
								if (nullptr != __pubkey) {
									len = BN_bn2bin(__pubkey, arrBN);
									if (0 < len) {
										pubKey.Resize((size_t) len);
										memcpy((void *) pubKey, arrBN, (size_t) len);
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

		int _ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const uint8_t *msg, size_t msglen,
									   size_t recid, size_t check) {
			if (!eckey) {
				return 0;
			}
			BN_CTX *ctx = nullptr;
			BIGNUM *x = nullptr, *e = nullptr, *order = nullptr, *sor = nullptr, *eor = nullptr, *field = nullptr;
			EC_POINT *R = nullptr, *O = nullptr, *Q = nullptr;
			BIGNUM *rr = nullptr, *zero = nullptr;
			size_t n = 0;
			int ret = 0;
			size_t i = recid / 2;
			const BIGNUM *r = nullptr;
			const BIGNUM *s = nullptr;
			ECDSA_SIG_get0(ecsig, &r, &s);
			const EC_GROUP *group = EC_KEY_get0_group(eckey);
			ctx = BN_CTX_new();
			if (!ctx) {
				ret = -1;
				goto err;
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
			if (!EC_GROUP_get_curve_GFp(group, field, nullptr, nullptr, ctx)) {
				ret = -2;
				goto err;
			}
			if (BN_cmp(x, field) >= 0) {
				ret = 0;
				goto err;
			}
			if ((R = EC_POINT_new(group)) == nullptr) {
				ret = -2;
				goto err;
			}
			if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) {
				ret = 0;
				goto err;
			}
			if (check) {
				if ((O = EC_POINT_new(group)) == nullptr) {
					ret = -2;
					goto err;
				}
				if (!EC_POINT_mul(group, O, nullptr, R, order, ctx)) {
					ret = -2;
					goto err;
				}
				if (!EC_POINT_is_at_infinity(group, O)) {
					ret = 0;
					goto err;
				}
			}
			if ((Q = EC_POINT_new(group)) == nullptr) {
				ret = -2;
				goto err;
			}
			n = EC_GROUP_get_degree(group);
			e = BN_CTX_get(ctx);
			if (!BN_bin2bn(msg, msglen, e)) {
				ret = -1;
				goto err;
			}
			if (8 * msglen > n) BN_rshift(e, e, 8 - (n & 7));
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
			if (R != nullptr) EC_POINT_free(R);
			if (O != nullptr) EC_POINT_free(O);
			if (Q != nullptr) EC_POINT_free(Q);

			return ret;
		}

		bool BTCKey::ECDSASign(const CMBlock &privKey, const CMBlock &data, CMBlock &signedData, int nid) {
			bool out = false;
			if (32 != privKey.GetSize() && 0 == data.GetSize()) {
				return out;
			}
			if (nid == NID_secp256k1) {
				BRKey key;
				if (1 == BRKeySetSecret(&key, (UInt256 *) (void *) privKey, 1)) {
					BRKeyPubKey(&key, nullptr, 0);
					UInt256 md = UINT256_ZERO;
					BRSHA256(&md, data, sizeof(data));
					uint8_t utSignedData[256] = {0};
					size_t szSignedLen = BRKeySign(&key, utSignedData, sizeof(utSignedData), md);
					signedData.Resize(szSignedLen);
					memcpy(signedData, utSignedData, szSignedLen);
					out = true;
				}
			} else {
				BIGNUM *_privkey = nullptr;
				if (32 != privKey.GetSize()) {
					return out;
				} else {
					_privkey = BN_bin2bn((const unsigned char *) (uint8_t *) privKey, (int) privKey.GetSize(), nullptr);
				}
				EC_KEY *key = EC_KEY_new_by_curve_name(nid);
				if (nullptr != _privkey && nullptr != key) {
					if (1 == EC_KEY_set_private_key(key, _privkey)) {
						BIGNUM *k = nullptr, *x = nullptr;
						const uint8_t *pPlainText = data;
						size_t szPlainText = data.GetSize();
						UInt256 md;
						BRSHA256(&md, pPlainText, szPlainText);
						if (1 == ECDSA_sign_setup(key, nullptr, &k, &x)) {
							ECDSA_SIG *sig = ECDSA_do_sign_ex((unsigned char *) &md, sizeof(md), k, x, key);
							if (nullptr != sig) {
								unsigned int nSize = ECDSA_size(key);
								signedData.Resize((size_t) nSize); // Make sure it is big enough
								unsigned char *pos = (unsigned char *) (void *) signedData;
								nSize = i2d_ECDSA_SIG(sig, &pos);
								signedData.Resize((size_t) nSize);
								ECDSA_SIG_free(sig);
								BN_free(k);
								BN_free(x);
								out = true;
							}
						}
					}
					EC_KEY_free(key);
					BN_free(_privkey);
				}
			}
			return out;
		}

		bool BTCKey::ECDSAVerify(const CMBlock &pubKey, const CMBlock &data, const CMBlock &signedData, int nid) {
			bool out = false;
			if (33 != pubKey.GetSize() || 0 == data.GetSize() || 0 == signedData.GetSize()) {
				return out;
			}
			if (nid == NID_secp256k1) {
				BRKey key;
				if (1 == BRKeySetPubKey(&key, pubKey, pubKey.GetSize())) {
					UInt256 md = UINT256_ZERO;
					BRSHA256(&md, data, sizeof(data));
					if (1 == BRKeyVerify(&key, md, signedData, signedData.GetSize())) {
						out = true;
					}
				}
			} else {
				BIGNUM *_pubkey = nullptr;
				if (33 != pubKey.GetSize()) {
					return out;
				} else {
					_pubkey = BN_bin2bn((const unsigned char *) (uint8_t *) pubKey, (int) pubKey.GetSize(), nullptr);
				}
				EC_KEY *key = EC_KEY_new_by_curve_name(nid);
				if (nullptr != _pubkey && nullptr != key) {
					const EC_GROUP *curve = EC_KEY_get0_group(key);
					EC_POINT *ec_p = EC_POINT_bn2point(curve, _pubkey, nullptr, nullptr);
					if (nullptr != ec_p) {
						if (1 == EC_KEY_set_public_key(key, ec_p)) {
							const uint8_t *pPlainText = data;
							size_t szPlainText = data.GetSize();
							UInt256 md;
							BRSHA256(&md, pPlainText, szPlainText);
							if (1 ==
								ECDSA_verify(0, (unsigned char *) &md, sizeof(md),
											 (unsigned char *) (void *) signedData,
											 (int) signedData.GetSize(), key)) {
								out = true;
							}
						}
						EC_POINT_free(ec_p);
					}
					EC_KEY_free(key);
					BN_free(_pubkey);
				} else {
					if (nullptr != _pubkey) {
						BN_free(_pubkey);
					}
					if (nullptr != key) {
						EC_KEY_free(key);
					}
				}
			}
			return out;
		}

		bool
		BTCKey::ECDSACompactSign(const CMBlock &privKey, const CMBlock &data, CMBlock &signedData, int nid) {
			bool out = false;
			if (32 != privKey.GetSize() || 0 == data.GetSize()) {
				return out;
			}
			if (nid == NID_secp256k1) {
				BRKey key;
				if (1 == BRKeySetSecret(&key, (UInt256 *) (void *) privKey, 1)) {
					BRKeyPubKey(&key, nullptr, 0);
					UInt256 md = UINT256_ZERO;
					BRSHA256(&md, data, sizeof(data));
					uint8_t utSignedData[256] = {0};
					size_t szSignedLen = BRKeyCompactSign(&key, utSignedData, sizeof(utSignedData), md);
					signedData.Resize(szSignedLen);
					memcpy(signedData, utSignedData, szSignedLen);
					out = true;
				}
			} else {
				EC_KEY *key = EC_KEY_new_by_curve_name(nid);
				if (key) {
					BIGNUM *_privkey = BN_bin2bn((const unsigned char *) (uint8_t *) privKey, (int) privKey.GetSize(),
												 nullptr);
					if (_privkey) {
						if (1 == EC_KEY_set_private_key(key, _privkey)) {
							UInt256 md = UINT256_ZERO;
							BRSHA256(&md, data, sizeof(data));
							ECDSA_SIG *sig = ECDSA_do_sign((unsigned char *) &md, sizeof(md), key);
							if (nullptr != sig) {
								const BIGNUM *r = nullptr;
								const BIGNUM *s = nullptr;
								ECDSA_SIG_get0(sig, &r, &s);
								int nBitsR = BN_num_bits(r);
								int nBitsS = BN_num_bits(s);
								bool fOk = false;
								int rec = -1;
								if (nBitsR <= 256 && nBitsS <= 256) {
									CMBlock pubKey = getPubKeyFromPrivKey(privKey, nid);
									for (int i = 0; i < 4; i++) {
										if (1 ==
											_ECDSA_SIG_recover_key_GFp(key, sig, (uint8_t *) &md, sizeof(md), i, 1)) {
											EC_KEY_set_conv_form(key, POINT_CONVERSION_COMPRESSED);
											int nSize = i2o_ECPublicKey(key, nullptr);
											assert(nSize);
											assert(nSize <= 65);
											unsigned char c[65];
											unsigned char *pbegin = c;
											nSize = i2o_ECPublicKey(key, &pbegin);
											if (0 == memcmp(c, pubKey, nSize)) {
												rec = i;
												fOk = true;
												break;
											}
										}
									}
								}
								if (fOk) {
									signedData.Resize(65);
									signedData.Zero();
									signedData[0] = 27 + rec + 4;
									uint8_t arrBIN[256] = {0};
									size_t szLen = 0;
									szLen = BN_bn2bin(r, arrBIN);
									memcpy(signedData + 1 + (32 - szLen), arrBIN, szLen);
									memset(arrBIN, 0, sizeof(arrBIN));
									szLen = BN_bn2bin(s, arrBIN);
									memcpy(signedData + 1 + 32 + (32 - szLen), arrBIN, szLen);
									out = true;
								}
								ECDSA_SIG_free(sig);
							}
						}
						BN_free(_privkey);
					}
					EC_KEY_free(key);
				}
			}
			return out;
		}

		bool
		BTCKey::ECDSACompactVerify(const CMBlock &pubKey, const CMBlock &data, const CMBlock &signedData, int nid) {
			bool out = false;
			if (33 != pubKey.GetSize() || 0 == data.GetSize() || 65 != signedData.GetSize()) {
				return out;
			}
			if (nid == NID_secp256k1) {
				BRKey key;
				UInt256 md = UINT256_ZERO;
				BRSHA256(&md, data, sizeof(data));
				if (1 == BRKeyRecoverPubKey(&key, md, signedData, signedData.GetSize())) {
					out = true;
				}
			} else {
				if (PublickeyIsValid(pubKey, nid)) {
					int recid = (signedData[0] - 27) & 3;
					if (recid < 0 || recid > 3) {
						return out;
					}
					BIGNUM *_pubkey = nullptr;
					_pubkey = BN_bin2bn((const unsigned char *) (uint8_t *) pubKey, (int) pubKey.GetSize(), nullptr);
					EC_KEY *key = EC_KEY_new_by_curve_name(nid);
					if (nullptr != _pubkey && nullptr != key) {
						const EC_GROUP *curve = EC_KEY_get0_group(key);
						EC_POINT *ec_p = EC_POINT_bn2point(curve, _pubkey, nullptr, nullptr);
						if (nullptr != ec_p) {
							if (1 == EC_KEY_set_public_key(key, ec_p)) {
								UInt256 md;
								BRSHA256(&md, data, data.GetSize());
								const uint8_t *p64 = &signedData[1];
								ECDSA_SIG *sig = ECDSA_SIG_new();
								if (nullptr != sig) {
									BIGNUM *r = BN_bin2bn(&p64[0], 32, nullptr);
									BIGNUM *s = BN_bin2bn(&p64[32], 32, nullptr);
									ECDSA_SIG_set0(sig, r, s);
									if (1 ==
										_ECDSA_SIG_recover_key_GFp(key, sig, (unsigned char *) &md, sizeof(md), recid,
																   0)) {
										int nSize = i2o_ECPublicKey(key, nullptr);
										assert(nSize);
										assert(nSize <= 65);
										unsigned char c[65];
										unsigned char *pbegin = c;
										nSize = i2o_ECPublicKey(key, &pbegin);
										if (0 == memcmp(c, pubKey, nSize)) {
											out = true;
										}
									}
									ECDSA_SIG_free(sig);
								}
							}
							EC_POINT_free(ec_p);
						}
						EC_KEY_free(key);
						BN_free(_pubkey);
					} else {
						if (nullptr != _pubkey) {
							BN_free(_pubkey);
						}
						if (nullptr != key) {
							EC_KEY_free(key);
						}
					}
				}
			}
			return out;
		}

		bool
		BTCKey::ECDSACompactSign_sha256(const CMBlock &privKey, const UInt256 &md, CMBlock &signedData,
										int nid) {
			bool out = false;
			if (32 != privKey.GetSize()) {
				return out;
			}
			if (nid == NID_secp256k1) {
				BRKey key;
				if (1 == BRKeySetSecret(&key, (UInt256 *) (void *) privKey, 1)) {
					BRKeyPubKey(&key, nullptr, 0);
					uint8_t utSignedData[256] = {0};
					size_t szSignedLen = BRKeyCompactSign(&key, utSignedData, sizeof(utSignedData), md);
					signedData.Resize(szSignedLen);
					memcpy(signedData, utSignedData, szSignedLen);
					out = true;
				}
			} else {
				EC_KEY *key = EC_KEY_new_by_curve_name(nid);
				if (key) {
					BIGNUM *_privkey = BN_bin2bn((const unsigned char *) (uint8_t *) privKey, (int) privKey.GetSize(),
												 nullptr);
					if (_privkey) {
						if (1 == EC_KEY_set_private_key(key, _privkey)) {
							ECDSA_SIG *sig = ECDSA_do_sign((unsigned char *) &md, sizeof(md), key);
							if (nullptr != sig) {
								const BIGNUM *r = nullptr;
								const BIGNUM *s = nullptr;
								ECDSA_SIG_get0(sig, &r, &s);
								int nBitsR = BN_num_bits(r);
								int nBitsS = BN_num_bits(s);
								bool fOk = false;
								int rec = -1;
								if (nBitsR <= 256 && nBitsS <= 256) {
									CMBlock pubKey = getPubKeyFromPrivKey(privKey, nid);
									for (int i = 0; i < 4; i++) {
										if (1 ==
											_ECDSA_SIG_recover_key_GFp(key, sig, (uint8_t *) &md, sizeof(md), i, 1)) {
											EC_KEY_set_conv_form(key, POINT_CONVERSION_COMPRESSED);
											int nSize = i2o_ECPublicKey(key, nullptr);
											assert(nSize);
											assert(nSize <= 65);
											unsigned char c[65];
											unsigned char *pbegin = c;
											nSize = i2o_ECPublicKey(key, &pbegin);
											if (0 == memcmp(c, pubKey, nSize)) {
												rec = i;
												fOk = true;
												break;
											}
										}
									}
								}
								if (fOk) {
									signedData.Resize(65);
									signedData.Zero();
									signedData[0] = 64;
									uint8_t arrBIN[256] = {0};
									size_t szLen = 0;
									szLen = BN_bn2bin(r, arrBIN);
									memcpy(signedData + 1 + (32 - szLen), arrBIN, szLen);
									memset(arrBIN, 0, sizeof(arrBIN));
									szLen = BN_bn2bin(s, arrBIN);
									memcpy(signedData + 1 + 32 + (32 - szLen), arrBIN, szLen);
									out = true;
								}
								ECDSA_SIG_free(sig);
							}
						}
						BN_free(_privkey);
					}
					EC_KEY_free(key);
				}
			}
			return out;
		}

		bool
		BTCKey::ECDSACompactVerify_sha256(const CMBlock &pubKey, const UInt256 &md, const CMBlock &signedData,
										  int nid) {
			bool out = false;
			if (33 != pubKey.GetSize() || 65 != signedData.GetSize()) {
				return out;
			}
			if (nid == NID_secp256k1) {
				BRKey key;
				if (1 == BRKeyRecoverPubKey(&key, md, signedData, signedData.GetSize())) {
					out = true;
				}
			} else {
				if (PublickeyIsValid(pubKey, nid)) {
					int recid = (signedData[0] - 27) & 3;
					if (recid < 0 || recid > 3) {
						return out;
					}
					BIGNUM *_pubkey = nullptr;
					_pubkey = BN_bin2bn((const unsigned char *) (uint8_t *) pubKey, (int) pubKey.GetSize(), nullptr);
					EC_KEY *key = EC_KEY_new_by_curve_name(nid);
					if (nullptr != _pubkey && nullptr != key) {
						const EC_GROUP *curve = EC_KEY_get0_group(key);
						EC_POINT *ec_p = EC_POINT_bn2point(curve, _pubkey, nullptr, nullptr);
						if (nullptr != ec_p) {
							if (1 == EC_KEY_set_public_key(key, ec_p)) {
								const uint8_t *p64 = &signedData[1];
								ECDSA_SIG *sig = ECDSA_SIG_new();
								if (nullptr != sig) {
									BIGNUM *r = BN_bin2bn(&p64[0], 32, nullptr);
									BIGNUM *s = BN_bin2bn(&p64[32], 32, nullptr);
									ECDSA_SIG_set0(sig, r, s);
									if (1 ==
										_ECDSA_SIG_recover_key_GFp(key, sig, (unsigned char *) &md, sizeof(md), recid,
																   0)) {
										EC_KEY_set_conv_form(key, POINT_CONVERSION_COMPRESSED);
										int nSize = i2o_ECPublicKey(key, nullptr);
										assert(nSize);
										assert(nSize <= 65);
										unsigned char c[65];
										unsigned char *pbegin = c;
										nSize = i2o_ECPublicKey(key, &pbegin);
										if (0 == memcmp(c, pubKey, nSize)) {
											out = true;
										}
									}
									ECDSA_SIG_free(sig);
								}
							}
							EC_POINT_free(ec_p);
						}
						EC_KEY_free(key);
						BN_free(_pubkey);
					} else {
						if (nullptr != _pubkey) {
							BN_free(_pubkey);
						}
						if (nullptr != key) {
							EC_KEY_free(key);
						}
					}
				}
			}
			return out;
		}

		typedef struct _ext_hd_t {

			unsigned char raw[SHA512_DIGEST_LENGTH];

		} ext_hd_t;

		typedef struct _hd_t {

			unsigned char first[SHA512_DIGEST_LENGTH >> 1];

			unsigned char second[SHA512_DIGEST_LENGTH >> 1];

		} hd_t;

		static void _BIP39DeriveKey(void *key64, const char *phrase, const char *passphrase, int nid) {
			OpenSSL_add_all_algorithms();

			ext_hd_t master_seed;
			char salt[strlen("mnemonic") + (passphrase ? strlen(passphrase) : 0) + 1];
			strcpy(salt, "mnemonic");
			if (passphrase) strcpy(salt + strlen("mnemonic"), passphrase);
			PKCS5_PBKDF2_HMAC(phrase, strlen(phrase), (unsigned char *) salt, strlen(salt), 2048, EVP_sha512(),
							  sizeof(master_seed.raw), master_seed.raw);

			ext_hd_t extended_private_key;
			size_t extended_private_keylen = sizeof(master_seed.raw);
			HMAC(EVP_sha512(), salt, strlen(salt), master_seed.raw, sizeof(master_seed.raw),
				 extended_private_key.raw,
				 (unsigned int *) &extended_private_keylen);
			memcpy(key64, extended_private_key.raw, extended_private_keylen);
		}

		CMBlock
		BTCKey::getPrivKeySeed(const std::string &phrase, const std::string &phrasePassword,
							   const boost::filesystem::path &i18nPath, const std::string &language, int nid) {
			CMBlock out;
			if (phrase == "" || i18nPath == "") {
				return out;
			}
			std::vector<std::string> words;
			if (language == "english" || language == "") {
				for (std::string str : BRBIP39WordsEn) {
					words.push_back(str);
				}
			} else {
				boost::filesystem::path fileName = i18nPath;
				fileName /= "mnemonic_" + language + ".txt";
				std::fstream infile(fileName.string());
				std::string line;
				while (std::getline(infile, line)) {
					words.push_back(line);
				}
			}
			CMemBlock<char> cbPhrase;
			cbPhrase.SetMemFixed(phrase.c_str(), phrase.size() + 1);
			if (BIP39_WORDLIST_COUNT == words.size()) {
				if (!WalletTool::PhraseIsValid(cbPhrase, words)) {
					return out;
				}
			}
			std::string prikeyBase58 = "";
			if (NID_secp256k1 == nid) {
				prikeyBase58 = WalletTool::getDeriveKey_base58(cbPhrase, phrasePassword);
			} else {
				prikeyBase58 = WalletTool::getDeriveKey_base58(cbPhrase, phrasePassword);
			}
			CMemBlock<unsigned char> prikey = BTCBase58::DecodeBase58(prikeyBase58);
			out.Resize(prikey.GetSize());
			memcpy(out, prikey, prikey.GetSize());
			return out;
		}

		CMBlock BTCKey::getPubKeyFromPrivKey(const CMBlock &privKey, int nid) {
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
					if (1 == EC_POINT_mul(curve, _pubkey, _privkey, nullptr, nullptr, nullptr)) {
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

		bool BTCKey::PublickeyIsValid(const CMBlock &pubKey, int nid) {
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

		bool BTCKey::KeyIsValid(const CMBlock &privKey, const CMBlock &pubKey, int nid) {
			bool out = false;
			if (0 == privKey.GetSize() || 0 == pubKey.GetSize()) {
				return out;
			}
			EC_KEY *key = EC_KEY_new_by_curve_name(nid);
			if (nullptr != key) {
				BIGNUM *pubkey = BN_bin2bn((const unsigned char *) (uint8_t *) pubKey, (int) pubKey.GetSize(),
										   nullptr);
				if (nullptr != pubkey) {
					const EC_GROUP *curve = EC_KEY_get0_group(key);
					EC_POINT *ec_p = EC_POINT_bn2point(curve, pubkey, nullptr, nullptr);
					if (nullptr != ec_p) {
						if (1 == EC_KEY_set_public_key(key, ec_p)) {
							BIGNUM *privkey = BN_bin2bn((const unsigned char *) (uint8_t *) privKey,
														(int) privKey.GetSize(), nullptr);
							if (nullptr != privkey) {
								if (1 == EC_KEY_set_private_key(key, privkey)) {
									if (1 == EC_KEY_check_key(key)) {
										out = true;
									}
								}
								BN_free(privkey);
							}
						}
						EC_POINT_free(ec_p);
					}
					BN_free(pubkey);
				}
				EC_KEY_free(key);
			}
			return out;
		}

		static CMBlock
		_ECPointAdd(const EC_GROUP *group, EC_POINT *point, const unsigned char vchTweak[32]) {
			CMBlock out;
			BN_CTX *ctx = BN_CTX_new();
			BN_CTX_start(ctx);
			BIGNUM *bnTweak = BN_CTX_get(ctx);
			BIGNUM *bnOrder = BN_CTX_get(ctx);
			BIGNUM *bnOne = BN_CTX_get(ctx);
			EC_GROUP_get_order(group, bnOrder,
							   ctx); // what a grossly inefficient way to get the (constant) group order...
			BN_bin2bn(vchTweak, 32, bnTweak);
			if (BN_cmp(bnTweak, bnOrder) >= 0)
				return out; // extremely unlikely
			BN_one(bnOne);
			EC_POINT_mul(group, point, bnTweak, point, bnOne, ctx);
			if (EC_POINT_is_at_infinity(group, point))
				return out; // ridiculously unlikely

			BIGNUM *pubkey = EC_POINT_point2bn(group, point, POINT_CONVERSION_COMPRESSED, nullptr, nullptr);
			if (nullptr != pubkey) {
				uint8_t arrBN[256] = {0};
				int len = BN_bn2bin(pubkey, arrBN);
				if (0 < len) {
					out.Resize(len);
					memcpy(out, arrBN, (size_t) len);
				}
				BN_free(pubkey);
			}

			BN_CTX_end(ctx);
			BN_CTX_free(ctx);
			return out;
		}

		static bool _TweakPublic(BRECPoint *K, const unsigned char vchTweak[32], int nid) {
			bool ret = false;
			EC_KEY *key = EC_KEY_new_by_curve_name(nid);
			if (nullptr != key) {
				BIGNUM *pubKey = BN_bin2bn((const unsigned char *) (uint8_t *) K->p, sizeof(K->p),
										   nullptr);
				if (nullptr != pubKey) {
					const EC_GROUP *curve = EC_KEY_get0_group(key);
					EC_POINT *ec_p = EC_POINT_bn2point(curve, pubKey, nullptr, nullptr);
					if (nullptr != ec_p) {
						if (1 == EC_KEY_set_public_key(key, ec_p)) {
							if (1 == EC_KEY_check_key(key)) {
								CMBlock mbDeriveKey = _ECPointAdd(curve, ec_p, vchTweak);
								if (true == mbDeriveKey && 33 == mbDeriveKey.GetSize()) {
									memcpy(K->p, mbDeriveKey, mbDeriveKey.GetSize());
									ret = true;
								}
							}
						}
						EC_POINT_free(ec_p);
					}
					BN_free(pubKey);
				}
				EC_KEY_free(key);
			}
			return ret;
		}

		static void _CKDpub(BRECPoint *K, UInt256 *c, uint32_t i, int nid) {
			uint8_t buf[sizeof(*K) + sizeof(i)];
			UInt512 I;

			if ((i & BIP32_HARD) != BIP32_HARD) { // can't derive private child key from public parent key
				*(BRECPoint *) buf = *K;
				UInt32SetBE(&buf[sizeof(*K)], i);

				BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf,
					   sizeof(buf)); // I = HMAC-SHA512(c, P(K) || i)
				*c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR
				if (NID_secp256k1 == nid) {
					BRSecp256k1PointAdd(K, (UInt256 *) &I); // K = P(IL) + K
				} else {
					_TweakPublic(K, (const unsigned char *) (UInt256 *) &I, nid);
				}

				var_clean(&I);
				mem_clean(buf, sizeof(buf));
			}
		}

		CMBlock
		BTCKey::getDerivePubKey(const CMBlock &pubKey, uint32_t chain, uint32_t index,
								UInt256 chainCode, int nid) {
			CMBlock out;
			if (true != pubKey ||
				33 != pubKey.GetSize()/* || !(chain == SEQUENCE_EXTERNAL_CHAIN || chain == SEQUENCE_INTERNAL_CHAIN)*/) {
				return out;
			}
			BRECPoint ecPoint;
			memcpy(ecPoint.p, pubKey, sizeof(ecPoint.p));
			_CKDpub(&ecPoint, &chainCode, chain, nid);
			_CKDpub(&ecPoint, &chainCode, index, nid);
			out.Resize(sizeof(ecPoint.p));
			memcpy(out, ecPoint.p, sizeof(ecPoint.p));
			return out;
		}

		static bool _TweakSecret(unsigned char vchSecretOut[32], const unsigned char vchSecretIn[32],
								 const unsigned char vchTweak[32], int nid) {
			bool ret = true;
			BN_CTX *ctx = BN_CTX_new();
			BN_CTX_start(ctx);
			BIGNUM *bnSecret = BN_CTX_get(ctx);
			BIGNUM *bnTweak = BN_CTX_get(ctx);
			BIGNUM *bnOrder = BN_CTX_get(ctx);
			EC_GROUP *group = EC_GROUP_new_by_curve_name(nid);
			EC_GROUP_get_order(group, bnOrder,
							   ctx); // what a grossly inefficient way to get the (constant) group order...
			BN_bin2bn(vchTweak, 32, bnTweak);
			if (BN_cmp(bnTweak, bnOrder) >= 0)
				ret = false; // extremely unlikely
			BN_bin2bn(vchSecretIn, 32, bnSecret);
			BN_add(bnSecret, bnSecret, bnTweak);
			BN_nnmod(bnSecret, bnSecret, bnOrder, ctx);
			if (BN_is_zero(bnSecret))
				ret = false; // ridiculously unlikely
			int nBits = BN_num_bits(bnSecret);
			memset(vchSecretOut, 0, 32);
			BN_bn2bin(bnSecret, &vchSecretOut[32 - (nBits + 7) / 8]);
			EC_GROUP_free(group);
			BN_CTX_end(ctx);
			BN_CTX_free(ctx);
			return ret;
		}

		static void _CKDpriv(UInt256 *k, UInt256 *c, uint32_t i, int nid) {
			uint8_t buf[sizeof(BRECPoint) + sizeof(i)];
			UInt512 I;

			if (i & BIP32_HARD) {
				buf[0] = 0;
				UInt256Set(&buf[1], *k);
			} else {
				if (NID_secp256k1 == nid) {
					BRSecp256k1PointGen((BRECPoint *) buf, k);
				} else {
					CMBlock privKey(sizeof(*k));
					memcpy(privKey, k, sizeof(*k));
					CMBlock pubKey;
					pubKey = BTCKey::getPubKeyFromPrivKey(privKey, nid);
					memcpy(buf, pubKey, pubKey.GetSize());
				}
			}

			UInt32SetBE(&buf[sizeof(BRECPoint)], i);

			BRHMAC(&I, BRSHA512, sizeof(UInt512), c, sizeof(*c), buf,
				   sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)

			if (NID_secp256k1 == nid) {
				BRSecp256k1ModAdd(k, (UInt256 *) &I); // k = IL + k (mod n)
			} else {
				UInt256 kOut;
				_TweakSecret((unsigned char *) &kOut.u8[0], (unsigned char *) &k->u8[0], (unsigned char *) &I.u8[0],
							 nid);
				*k = kOut;
			}
			*c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR

			var_clean(&I);
			mem_clean(buf, sizeof(buf));
		}

		// sets the private key for the path specified by vlist to key
		// depth is the number of arguments in vlist
		static void
		_BRBIP32vPrivKeyPath(BRKey *key, int nid, bool bSecret, UInt256 &chainCode, bool useChainCode,
							 const void *seed, size_t seedLen, int depth, va_list vlist) {
			UInt512 I;
			UInt256 secret, s = UINT256_ZERO, c = UINT256_ZERO, cmp = UINT256_ZERO;

			assert(key != nullptr);
			assert(seed != nullptr || seedLen == 0);
			assert(depth >= 0);

			if (key && (seed || seedLen == 0)) {
				if (!bSecret) {
					BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
					secret = *(UInt256 *) &I;
					chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
					var_clean(&I);
				}

				for (int i = 0; i < depth; i++) {
					if (2 > i) {
						if (!bSecret) {
							_CKDpriv(&secret, &chainCode, va_arg(vlist, uint32_t), nid);
							BRKeySetSecret(key, &secret, 1);
						} else {
							va_arg(vlist, uint32_t);
						}
					} else {
						if (!useChainCode && 0 != memcmp(&chainCode, &cmp, sizeof(UInt256))) {
							chainCode = UINT256_ZERO;
						}
						if (0 == memcmp(&s, &cmp, sizeof(UInt256)) && 0 == memcmp(&c, &cmp, sizeof(UInt256))) {
							if (!bSecret) {
								s = secret;
							} else {
								assert(seedLen == sizeof(UInt256));
								memcpy(&s, seed, sizeof(UInt256));
							}
							c = chainCode;
						}
						uint32_t pm = va_arg(vlist, uint32_t);
						_CKDpriv(&s, &c, pm, nid);
						BRKeySetSecret(key, &s, 1);
					}
				}

				if (!useChainCode) {
					var_clean(&secret, &chainCode, &c, &s, &cmp);
				} else {
					var_clean(&secret, &c, &s, &cmp);
				}
			}
		}

		// sets the private key for the specified path to key
		// depth is the number of arguments used to specify the path
		static void
		_BRBIP32PrivKeyPath(BRKey *key, int nid, bool bSecret, UInt256 &chainCode, bool useChainCode,
							const void *seed, size_t seedLen, int depth, ...) {
			va_list ap;
			va_start(ap, depth);
			_BRBIP32vPrivKeyPath(key, nid, bSecret, chainCode, useChainCode, seed, seedLen, depth, ap);
			va_end(ap);
		}

		// sets the private key for path m/0H/chain/index to key
		static void
		_BRBIP32PrivKey(BRKey *key, int nid, bool bSecret, UInt256 &chainCode, bool useChainCode, const void *seed,
						size_t seedLen, uint32_t chain, uint32_t index) {
			_BRBIP32PrivKeyPath(key, nid, bSecret, chainCode, useChainCode, seed, seedLen, 4, 1 | BIP32_HARD, chain,
								chain, index);
		}

		CMBlock
		BTCKey::getDerivePrivKey(const CMBlock &seed, uint32_t chain, uint32_t index, UInt256 &chainCode,
								 bool useChainCode, int nid) {
			CMBlock out;
			if (0 == seed.GetSize()) {
				return out;
			}
			BRKey key;
			_BRBIP32PrivKey(&key, nid, false, chainCode, useChainCode, seed, seed.GetSize(), chain, index);
			out.Resize(sizeof(key.secret));
			memcpy(out, &key.secret.u8[0], sizeof(key.secret));
			return out;
		}

		CMBlock
		BTCKey::getDerivePrivKey_depth(const CMBlock &seed, UInt256 &chainCode, bool useChainCode, int nid,
									   int depth, ...) {
			CMBlock out;
			if (0 == seed.GetSize()) {
				return out;
			}
			BRKey key;
			va_list ap;
			va_start(ap, depth);
			_BRBIP32vPrivKeyPath(&key, nid, false, chainCode, useChainCode, seed, seed.GetSize(), depth, ap);
			va_end(ap);
			out.Resize(sizeof(key.secret));
			memcpy(out, &key.secret.u8[0], sizeof(key.secret));
			return out;
		}

		CMBlock
		BTCKey::getDerivePrivKey_depth(const CMBlock &seed, UInt256 &chainCode, bool useChainCode, int nid,
									   int depth, va_list ap) {
			CMBlock out;
			if (0 == seed.GetSize()) {
				return out;
			}
			BRKey key;
			_BRBIP32vPrivKeyPath(&key, nid, false, chainCode, useChainCode, seed, seed.GetSize(), depth, ap);
			out.Resize(sizeof(key.secret));
			memcpy(out, &key.secret.u8[0], sizeof(key.secret));
			return out;
		}

		CMBlock
		BTCKey::getDerivePrivKey_Secret(const CMBlock &privKey, uint32_t chain, uint32_t index,
										UInt256 chainCode, int nid) {
			CMBlock out;
			if (32 != privKey.GetSize()) {
				return out;
			}
			BRKey key;
			_BRBIP32PrivKey(&key, nid, true, chainCode, true, privKey, privKey.GetSize(), chain, index);
			out.Resize(sizeof(key.secret));
			memcpy(out, &key.secret.u8[0], sizeof(key.secret));
			return out;
		}

		CMBlock
		BTCKey::getDerivePrivKey_Secret_depth(const CMBlock &privKey, UInt256 chainCode, bool useChainCode,
											  int nid, int depth, ...) {
			CMBlock out;
			if (32 != privKey.GetSize()) {
				return out;
			}
			BRKey key;
			va_list ap;
			va_start(ap, depth);
			_BRBIP32vPrivKeyPath(&key, nid, true, chainCode, useChainCode, privKey, privKey.GetSize(), depth, ap);
			va_end(ap);
			out.Resize(sizeof(key.secret));
			memcpy(out, &key.secret.u8[0], sizeof(key.secret));
			return out;
		}

		CMBlock
		BTCKey::getDerivePrivKey_Secret_depth(const CMBlock &privKey, UInt256 chainCode, bool useChainCode,
											  int nid, int depth, va_list ap) {
			CMBlock out;
			if (32 != privKey.GetSize()) {
				return out;
			}
			BRKey key;
			_BRBIP32vPrivKeyPath(&key, nid, true, chainCode, useChainCode, privKey, privKey.GetSize(), depth, ap);
			out.Resize(sizeof(key.secret));
			memcpy(out, &key.secret.u8[0], sizeof(key.secret));
			return out;
		}

		// sets the private key for path m/0H/chain/index to each element in keys
		void _BRBIP32PrivKeyList(BRKey keys[], size_t keysCount, bool bSecret, const void *seed, size_t seedLen,
								 uint32_t chain, const uint32_t indexes[], UInt256 &chainCode, bool useChainCode,
								 int nid) {
			UInt512 I;
			UInt256 secret, s, c;

			assert(keys != nullptr || keysCount == 0);
			assert(seed != nullptr || seedLen == 0);
			assert(indexes != nullptr || keysCount == 0);

			if (keys && keysCount > 0 && (seed || seedLen == 0) && indexes) {
				if (!bSecret) {
					BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seedLen);
					secret = *(UInt256 *) &I;
					chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];
					var_clean(&I);

					_CKDpriv(&secret, &chainCode, 1 | BIP32_HARD, nid); // path m/0H
					_CKDpriv(&secret, &chainCode, chain, nid); // path m/0H/chain
				} else {
					assert(seedLen == sizeof(UInt256));
					memcpy(&secret, seed, sizeof(UInt256));
				}

				if (!useChainCode) {
					chainCode = UINT256_ZERO;
				}

				for (size_t i = 0; i < keysCount; i++) {
					s = secret;
					c = chainCode;
					_CKDpriv(&s, &c, chain, nid);//add addition
					_CKDpriv(&s, &c, indexes[i], nid); // index'th key in chain
					BRKeySetSecret(&keys[i], &s, 1);
				}

				if (!useChainCode) {
					var_clean(&secret, &chainCode, &c, &s);
				} else {
					var_clean(&secret, &c, &s);
				}
			}
		}

		void
		BTCKey::getDerivePrivKey(std::vector<CMBlock> &privKeys, const CMBlock &seed,
								 uint32_t chain, const uint32_t indexes[], UInt256 &chainCode,
								 bool useChainCode, int nid) {
			if (0 == privKeys.size() || 0 == seed.GetSize() || !indexes) {
				return;
			}
			size_t keysCount = privKeys.size();
			BRKey keys[keysCount];
			memset(keys, 0, sizeof(keys));
			_BRBIP32PrivKeyList(keys, keysCount, false, seed, seed.GetSize(), chain, indexes, chainCode,
								useChainCode,
								nid);
			for (size_t i = 0; i < keysCount; i++) {
				CMBlock mbSecret(sizeof(keys[i].secret));
				memcpy(mbSecret, &keys[i].secret, sizeof(keys[i].secret));
				privKeys[i] = mbSecret;
			}
		}

		void
		BTCKey::getDerivePrivKey_Secret(std::vector<CMBlock> &privKeys, const CMBlock &privKey,
										uint32_t chain, const uint32_t indexes[], UInt256 chainCode,
										int nid) {
			if (0 == privKeys.size() || 32 != privKey.GetSize() || !indexes) {
				return;
			}
			size_t keysCount = privKeys.size();
			BRKey keys[keysCount];
			memset(keys, 0, sizeof(keys));
			_BRBIP32PrivKeyList(keys, keysCount, true, privKey, privKey.GetSize(), chain, indexes, chainCode, true,
								nid);
			for (size_t i = 0; i < keysCount; i++) {
				CMBlock mbSecret(sizeof(keys[i].secret));
				memcpy(mbSecret, &keys[i].secret, sizeof(keys[i].secret));
				privKeys[i] = mbSecret;
			}
		}

		// key used for authenticated API calls, i.e. bitauth: https://github.com/bitpay/bitauth - path m/1H/0
		void _BRBIP32APIAuthKey(BRKey *key, UInt256 &chainCode, const void *seed, size_t seedLen, int nid) {
			_BRBIP32PrivKeyPath(key, nid, false, chainCode, true, seed, seedLen, 2, 1 | BIP32_HARD, 0);
		}

		CMBlock
		BTCKey::getMasterPrivkey(const CMBlock &seed, int nid) {
			CMBlock out;
			if (0 == seed.GetSize()) {
				return out;
			}
			UInt256 chainCode = UINT256_ZERO;
			BRKey key;
			if (NID_secp256k1 == nid) {
				_BRBIP32APIAuthKey(&key, chainCode, seed, seed.GetSize(), nid);
				out.Resize(sizeof(key.secret));
				memcpy(out, &key.secret, sizeof(key.secret));
			} else {
				_BRBIP32APIAuthKey(&key, chainCode, seed, seed.GetSize(), nid);
				out.Resize(sizeof(key.secret));
				memcpy(out, &key.secret, sizeof(key.secret));
			}
			return out;
		}
	}
}
