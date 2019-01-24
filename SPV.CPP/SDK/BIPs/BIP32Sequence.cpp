// Created by Aaron Voisine on 8/19/15.
// Copyright (c) 2015 breadwallet LLC
// Copyright (c) 2017-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
#include "BIP32Sequence.h"

namespace Elastos {
	namespace ElaWallet {

#define BIP32_SEED_KEY "Bitcoin seed"
#define BIP32_XPRV     "\x04\x88\xAD\xE4"
#define BIP32_XPUB     "\x04\x88\xB2\x1E"

		MasterPubKey BIP32Sequence::GetMasterPubKey(const CMBlock &seed) {
			MasterPubKey mpk;
			UInt512 I;
			UInt256 secret, chain;

			BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seed.GetSize());
			secret = *(UInt256 *) &I;
			chain = *(UInt256 *) &I.u8[sizeof(UInt256)];
			var_clean(&I);

			CMBlock pubKey = PrivKeyToPubKey(secret);

			UInt160 hash;
			BRHash160(&hash, pubKey, pubKey.GetSize());
			mpk.SetFingerPrint(hash.u32[0]);

			CKDpriv(secret, chain, 0 | BIP32_HARD); // path m/0H

			mpk.SetChainCode(chain);
			pubKey = PrivKeyToPubKey(secret);
			mpk.SetPubKey(pubKey); // path N(m/0H)
			var_clean(&secret, &chain);

			return mpk;
		}

		CMBlock BIP32Sequence::PubKey(const MasterPubKey &mpk, uint32_t change, uint32_t index) {
			assert(!mpk.Empty());

			UInt256 chainCode = mpk.GetChainCode();
			CMBlock pubKey = mpk.GetPubKey();
			ECPoint point;
			memcpy(point.p, pubKey, pubKey.GetSize() < sizeof(point.p) ? pubKey.GetSize() : sizeof(point.p));

			CKDpub(point, chainCode, change);
			CKDpub(point, chainCode, index);

			var_clean(&chainCode);

			return CMBlock(point.p, sizeof(point.p));
		}

		Key BIP32Sequence::PrivKey(const CMBlock &seed, uint32_t change, uint32_t index) {
			UInt256 chainCode;
			return PrivKeyPath(chainCode, seed, 3, 0 | BIP32_HARD, change, index);
		}

		std::vector<Key> BIP32Sequence::PrivKeyList(const CMBlock &seed, uint32_t change, const std::vector<uint32_t> &indexes) {
			UInt512 I;
			UInt256 secret, chainCode, s, c;

			BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seed.GetSize());
			secret = *(UInt256 *) &I;
			chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];

			var_clean(&I);

			CKDpriv(secret, chainCode, 0 | BIP32_HARD);
			CKDpriv(secret, chainCode, change);

			std::vector<Key> keys;
			for (int i = 0; i < indexes.size(); ++i) {
				s = secret;
				c = chainCode;
				CKDpriv(s, c, indexes[i]);
				keys.emplace_back(s, true);
			}

			var_clean(&secret, &chainCode, &c, &s);

			return keys;
		}

		// BIP44
		std::vector<Key> BIP32Sequence::PrivKeyList(const void *seed, size_t seedLen, uint32_t coinType, uint32_t change,
											const std::vector<uint32_t> &indexes) {
			UInt256 chainCode;
			uint32_t account = 0;

			std::vector<Key> keys;
			for (int i = 0; i < indexes.size(); ++i) {
				Key key = PrivKeyPath(chainCode, seed, 5, 44 | BIP32_HARD, coinType | BIP32_HARD, account | BIP32_HARD,
									  change, indexes[i]);
				keys.push_back(key);
			}

			return keys;
		}

		CMBlock BIP32Sequence::PubKeyPath(const MasterPubKey &mpk, int depth, ...) {
			va_list ap;

			va_start(ap, depth);
			CMBlock pubKey = PubKeyvPath(mpk, depth, ap);
			va_end(ap);

			return pubKey;
		}

		CMBlock BIP32Sequence::PubKeyvPath(const MasterPubKey &mpk, int depth, va_list vlist) {
			assert(!mpk.Empty());

			UInt256 chainCode = mpk.GetChainCode();
			CMBlock pubKey = mpk.GetPubKey();
			ECPoint point;
			memcpy(point.p, pubKey, pubKey.GetSize() < sizeof(point.p) ? pubKey.GetSize() : sizeof(point.p));

			for (int i = 0; i < depth; ++i) {
				CKDpub(point, chainCode, va_arg(vlist, uint32_t));
			}

			var_clean(&chainCode);

			return CMBlock(point.p, sizeof(point.p));
		}

		Key BIP32Sequence::PrivKeyPath(const void *seed, size_t seedLen, UInt256 &chainCode, int depth, ...) {
			va_list ap;
			CMBlock seedData;
			seedData.SetMemFixed(seed, seedLen);

			va_start(ap, depth);
			Key key = PrivKeyvPath(chainCode, seedData, depth, ap);
			va_end(ap);

			return key;
		}

		Key BIP32Sequence::PrivKeyPath(UInt256 &chainCode, const CMBlock &seed, int depth, ...) {
			va_list ap;

			va_start(ap, depth);
			Key key = PrivKeyvPath(chainCode, seed, depth, ap);
			va_end(ap);

			return key;
		}

		Key BIP32Sequence::PrivKeyvPath(UInt256 &chainCode, const CMBlock &seed, int depth, va_list vlist) {
			UInt512 I;
			UInt256 secret;

			BRHMAC(&I, BRSHA512, sizeof(UInt512), BIP32_SEED_KEY, strlen(BIP32_SEED_KEY), seed, seed.GetSize());

			secret = *(UInt256 *) &I;
			chainCode = *(UInt256 *) &I.u8[sizeof(UInt256)];

			var_clean(&I);

			for (int i = 0; i < depth; ++i) {
				CKDpriv(secret, chainCode, va_arg(vlist, uint32_t));
			}

			return Key(secret, true);
		}

		// key used for authenticated API calls, i.e. bitauth: https://github.com/bitpay/bitauth - path m/1H/0
		Key BIP32Sequence::APIAuthKey(const void *seed, size_t seedLen) {
			UInt256 chainCode;
			return PrivKeyPath(seed, seedLen, chainCode, 2, 1 | BIP32_HARD, 0);
		}

		// key used for BitID: https://github.com/bitid/bitid/blob/master/BIP_draft.md
		Key BIP32Sequence::BitIDKey(const CMBlock &seed, uint32_t index, const std::string &uri) {
			UInt256 chainCode;
			UInt256 hash;
			ByteStream stream;

			stream.writeUint32(index);
			stream.writeBytes(uri.data(), uri.length());

			CMBlock data = stream.getBuffer();

			BRSHA256(&hash, data, data.GetSize());

			return PrivKeyPath(chainCode, seed, 5, 13 | BIP32_HARD, hash.u32[0] | BIP32_HARD, hash.u32[1] | BIP32_HARD,
							   hash.u32[2] | BIP32_HARD, hash.u32[3] | BIP32_HARD);
		}

		CMBlock BIP32Sequence::PrivKeyToPubKey(const UInt256 &k) {
			CMBlock pubKey;

			BIGNUM *bnk = BN_bin2bn((const unsigned char *)&k, sizeof(k), nullptr);
			if (bnk == nullptr) {
				return pubKey;
			}

			EC_KEY *key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
			if (key != nullptr) {
				const EC_GROUP *curve = EC_KEY_get0_group(key);
				EC_POINT *r = EC_POINT_new(curve);
				if (r != nullptr) {
					if (EC_POINT_mul(curve, r, bnk, nullptr, nullptr, nullptr)) {
						BIGNUM *point = EC_POINT_point2bn(curve, r, POINT_CONVERSION_COMPRESSED, nullptr, nullptr);
						if (point != nullptr) {
							uint8_t bn[256] = {0};
							int len = BN_bn2bin(point, bn);
							pubKey = CMBlock(bn, len);
							BN_free(point);
						}
					}
					EC_POINT_free(r);
				}

				EC_KEY_free(key);
			}

			BN_free(bnk);

			return pubKey;
		}

		bool BIP32Sequence::ECPointAdd(const EC_GROUP *group, EC_POINT *point, const UInt256 &tweak, ECPoint &k) {
			BN_CTX *ctx = BN_CTX_new();
			if (ctx == nullptr) {
				Log::error("EC point add fail: new ctx");
				return false;
			}

			BN_CTX_start(ctx);
			BIGNUM *bnTweak = BN_CTX_get(ctx);
			BIGNUM *bnOrder = BN_CTX_get(ctx);
			BIGNUM *bnOne = BN_CTX_get(ctx);

			// what a grossly inefficient way to get the (constant) group order...
			EC_GROUP_get_order(group, bnOrder, ctx);
			BN_bin2bn(tweak.u8, sizeof(tweak), bnTweak);
			if (BN_cmp(bnTweak, bnOrder) >= 0) {
				Log::error("EC point add fail: BN tweak >= BN order");
				BN_CTX_end(ctx);
				BN_CTX_free(ctx);
				return false; // extremely unlikely
			}

			BN_one(bnOne);
			EC_POINT_mul(group, point, bnTweak, point, bnOne, ctx);
			if (EC_POINT_is_at_infinity(group, point)) {
				Log::error("EC point add fail: point is infinity");
				BN_CTX_end(ctx);
				BN_CTX_free(ctx);
				return false; // ridiculously unlikely
			}

			BIGNUM *bnPoint = EC_POINT_point2bn(group, point, POINT_CONVERSION_COMPRESSED, nullptr, nullptr);
			if (nullptr != bnPoint) {
				uint8_t bin[256] = {0};
				int len = BN_bn2bin(bnPoint, bin);
				assert(len == sizeof(k.p));
				if (0 < len) {
					memcpy(k.p, bin, len < sizeof(k.p) ? len : sizeof(k.p));
				} else {
					Log::error("EC point add fail: bn2bin");
				}
				BN_free(bnPoint);
			}

			BN_CTX_end(ctx);
			BN_CTX_free(ctx);

			return true;
		}

		bool BIP32Sequence::TweakPublic(ECPoint &k, const UInt256 &tweak, int nid) {
			bool result = false;
			EC_KEY *key = EC_KEY_new_by_curve_name(nid);
			if (key == nullptr) {
				Log::error("Tweak public fail: key is null");
				return false;
			}

			BIGNUM *bnPubKey = BN_bin2bn(k.p, sizeof(k.p), nullptr);
			if (nullptr != bnPubKey) {
				const EC_GROUP *curve = EC_KEY_get0_group(key);
				EC_POINT *ecPoint = EC_POINT_bn2point(curve, bnPubKey, nullptr, nullptr);
				if (nullptr != ecPoint) {
					if (1 == EC_KEY_set_public_key(key, ecPoint)) {
						if (1 == EC_KEY_check_key(key)) {
							if (ECPointAdd(curve, ecPoint, tweak, k)) {
								result = true;
							}
						}
					}
					EC_POINT_free(ecPoint);
				}
				BN_free(bnPubKey);
			}
			EC_KEY_free(key);

			return result;
		}

		void BIP32Sequence::CKDpub(ECPoint &k, UInt256 &c, uint32_t i) {
			uint8_t buf[sizeof(k) + sizeof(i)];
			UInt512 I;

			if ((i & BIP32_HARD) != BIP32_HARD) { // can't derive private child key from public parent key
				*(ECPoint *) buf = k;
				UInt32SetBE(&buf[sizeof(k)], i);

				BRHMAC(&I, BRSHA512, sizeof(UInt512), &c, sizeof(c), buf, sizeof(buf)); // I = HMAC-SHA512(c, P(K) || i)

				c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR
				TweakPublic(k, *(UInt256 *) &I, NID_X9_62_prime256v1);

				var_clean(&I);
				mem_clean(buf, sizeof(buf));
			}
		}

		void BIP32Sequence::CKDpriv(UInt256 &k, UInt256 &c, uint32_t i) {
			uint8_t buf[sizeof(ECPoint) + sizeof(i)];
			UInt512 I;

			if (i & BIP32_HARD) {
				buf[0] = 0;
				UInt256Set(&buf[1], k);
			} else {
				CMBlock pubKey = PrivKeyToPubKey(k);
				assert(pubKey.GetSize() == sizeof(ECPoint));
				memcpy(buf, pubKey, pubKey.GetSize());
			}

			UInt32SetBE(&buf[sizeof(ECPoint)], i);

			BRHMAC(&I, BRSHA512, sizeof(UInt512), &c, sizeof(c), buf, sizeof(buf)); // I = HMAC-SHA512(c, k|P(k) || i)

			TweakSecret(k, *(UInt256 *) &I, NID_X9_62_prime256v1);
			c = *(UInt256 *) &I.u8[sizeof(UInt256)]; // c = IR

			var_clean(&I);
			mem_clean(buf, sizeof(buf));
		}

		bool BIP32Sequence::TweakSecret(UInt256 &k, const UInt256 &tweak, int nid) {
			BN_CTX *ctx = BN_CTX_new();
			if (ctx == nullptr) {
				Log::error("Tweak secret error");
				return false;
			}

			BN_CTX_start(ctx);

			BIGNUM *bnSecret = BN_CTX_get(ctx);
			BIGNUM *bnTweak = BN_CTX_get(ctx);
			BIGNUM *bnOrder = BN_CTX_get(ctx);
			EC_GROUP *group = EC_GROUP_new_by_curve_name(nid);
			if (group == nullptr) {
				BN_CTX_end(ctx);
				BN_CTX_free(ctx);
				return false;
			}

			// what a grossly inefficient way to get the (constant) group order...
			EC_GROUP_get_order(group, bnOrder, ctx);
			BN_bin2bn(tweak.u8, sizeof(tweak), bnTweak);
			if (BN_cmp(bnTweak, bnOrder) >= 0) {
				Log::error("Tweak secret error: tweak >= order");
				EC_GROUP_free(group);
				BN_CTX_end(ctx);
				BN_CTX_free(ctx);
				return false; // extremely unlikely
			}

			BN_bin2bn(k.u8, sizeof(k), bnSecret);
			BN_add(bnSecret, bnSecret, bnTweak);
			BN_nnmod(bnSecret, bnSecret, bnOrder, ctx);

			if (BN_is_zero(bnSecret)) {
				Log::error("Tweak secret error: k is zero");
				EC_GROUP_free(group);
				BN_CTX_end(ctx);
				BN_CTX_free(ctx);
				return false; // ridiculously unlikely
			}

			int nBits = BN_num_bits(bnSecret);
			memset(k.u8, 0, sizeof(k));
			BN_bn2bin(bnSecret, &k.u8[sizeof(k) - (nBits + 7) / 8]);

			EC_GROUP_free(group);
			BN_CTX_end(ctx);
			BN_CTX_free(ctx);

			return true;
		}

	}
}