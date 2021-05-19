////////////////////////////////////////////////////////////////////////////////
//
// hdkeys.cpp
//
// Copyright (c) 2013-2014 Eric Lombrozo
// Copyright (c) 2011-2016 Ciphrex Corp.
//
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.
//

#include "HDKeychain.h"
#include "secp256k1_openssl.h"

#include <Common/hash.h>
#include <Common/BigInt.h>
#include <Common/uchar_vector.h>
#include <Common/typedefs.h>
#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/ErrorChecker.h>

#include <sstream>
#include <stdexcept>


namespace Elastos {
	namespace ElaWallet {

		const uchar_vector CURVE_ORDER_BYTES("FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551");
		const BigInt CURVE_ORDER(CURVE_ORDER_BYTES);

		const uint32_t BITCOIN_HD_PRIVATE_VERSION = 0x0488ade4;
		const uint32_t BITCOIN_HD_PUBLIC_VERSION = 0x0488b21e;

		bytes_t HDSeed::getExtendedKey(bool bPrivate) const {
			HDKeychain keychain(master_key_, master_chain_code_);
			if (!bPrivate) { keychain = keychain.getPublic(); }

			return keychain.extkey();
		}

		HDKeychain::HDKeychain(const bytes_t &key, const bytes_t &chain_code, uint32_t child_num, uint32_t parent_fp,
				uint32_t depth)
			: _depth(depth), _parent_fp(parent_fp), _child_num(child_num), _chain_code(chain_code), _key(key) {
				ErrorChecker::CheckLogic(_chain_code.size() != 32, Error::Key, "Invalid chain code.");

				if (_key.size() == 32) {
					// key is private
					BigInt n(_key);
					if (n >= CURVE_ORDER || n.isZero()) {
						ErrorChecker::ThrowLogicException(Error::Key, "Invalid key.");
					}

					uchar_vector privkey;
					privkey.push_back(0x00);
					privkey += _key;
					_key = privkey;
				} else if (_key.size() == 33) {
					// key is public
					secp256k1_point K(_key);
				} else {
					ErrorChecker::ThrowLogicException(Error::Key, "Invalid key.");
				}

				_version = isPrivate() ? _priv_version : _pub_version;
				updatePubkey();

				_valid = true;
			}

		HDKeychain::HDKeychain(const bytes_t &extkey) {
			ErrorChecker::CheckLogic(extkey.size() != 78, Error::Key, "Invalid extended key length.");

			_version = ((uint32_t) extkey[0] << 24) | ((uint32_t) extkey[1] << 16) | ((uint32_t) extkey[2] << 8) |
				(uint32_t) extkey[3];
			_depth = extkey[4];
			_parent_fp = ((uint32_t) extkey[5] << 24) | ((uint32_t) extkey[6] << 16) | ((uint32_t) extkey[7] << 8) |
				(uint32_t) extkey[8];
			_child_num = ((uint32_t) extkey[9] << 24) | ((uint32_t) extkey[10] << 16) | ((uint32_t) extkey[11] << 8) |
				(uint32_t) extkey[12];
			_chain_code.assign(extkey.begin() + 13, extkey.begin() + 45);
			_key.assign(extkey.begin() + 45, extkey.begin() + 78);

			updatePubkey();

			_valid = true;
		}

		HDKeychain::HDKeychain(const HDKeychain &source) {
			_valid = source._valid;
			if (!_valid) return;

			_version = source._version;
			_depth = source._depth;
			_parent_fp = source._parent_fp;
			_child_num = source._child_num;
			_chain_code = source._chain_code;
			_key = source._key;
			updatePubkey();
		}

		HDKeychain &HDKeychain::operator=(const HDKeychain &rhs) {
			_valid = rhs._valid;
			if (_valid) {
				_version = rhs._version;
				_depth = rhs._depth;
				_parent_fp = rhs._parent_fp;
				_child_num = rhs._child_num;
				_chain_code = rhs._chain_code;
				_key = rhs._key;
				updatePubkey();
			}
			return *this;
		}

		bool HDKeychain::operator==(const HDKeychain &rhs) const {
			return (_valid && rhs._valid &&
					_version == rhs._version &&
					_depth == rhs._depth &&
					_parent_fp == rhs._parent_fp &&
					_child_num == rhs._child_num &&
					_chain_code == rhs._chain_code &&
					_key == rhs._key);
		}

		bool HDKeychain::operator!=(const HDKeychain &rhs) const {
			return !(*this == rhs);
		}

		bytes_t HDKeychain::extkey() const {
			uchar_vector extkey;

			extkey.push_back((uint32_t) _version >> 24);
			extkey.push_back(((uint32_t) _version >> 16) & 0xff);
			extkey.push_back(((uint32_t) _version >> 8) & 0xff);
			extkey.push_back((uint32_t) _version & 0xff);

			extkey.push_back(_depth);

			extkey.push_back((uint32_t) _parent_fp >> 24);
			extkey.push_back(((uint32_t) _parent_fp >> 16) & 0xff);
			extkey.push_back(((uint32_t) _parent_fp >> 8) & 0xff);
			extkey.push_back((uint32_t) _parent_fp & 0xff);

			extkey.push_back((uint32_t) _child_num >> 24);
			extkey.push_back(((uint32_t) _child_num >> 16) & 0xff);
			extkey.push_back(((uint32_t) _child_num >> 8) & 0xff);
			extkey.push_back((uint32_t) _child_num & 0xff);

			extkey += _chain_code;
			extkey += _key;

			return extkey;
		}

		bytes_t HDKeychain::privkey() const {
			if (isPrivate()) {
				return bytes_t(_key.begin() + 1, _key.end());
			} else {
				return bytes_t();
			}
		}

		bytes_t HDKeychain::uncompressed_pubkey() const {
			secp256k1_key key;
			key.setPubKey(_pubkey);
			return key.getPubKey(false);
		}

		bytes_t HDKeychain::hash() const {
			return ripemd160(sha256(_pubkey));
		}

		bytes_t HDKeychain::full_hash() const {
			uchar_vector_secure data(_pubkey);
			data += _chain_code;
			return ripemd160(sha256(data));
		}

		uint32_t HDKeychain::fp() const {
			bytes_t hash = this->hash();
			return (uint32_t) hash[0] << 24 | (uint32_t) hash[1] << 16 | (uint32_t) hash[2] << 8 | (uint32_t) hash[3];
		}

		HDKeychain HDKeychain::getPublic() const {
			ErrorChecker::CheckLogic(!_valid, Error::Key, "invalid hd keychain");

			HDKeychain pub;
			pub._valid = _valid;
			pub._version = _pub_version;
			pub._depth = _depth;
			pub._parent_fp = _parent_fp;
			pub._child_num = _child_num;
			pub._chain_code = _chain_code;
			pub._key = pub._pubkey = _pubkey;
			return pub;
		}

		HDKeychain HDKeychain::getChild(uint32_t i) const {
			ErrorChecker::CheckLogic(!_valid, Error::Key, "invalid hd keychain");

			bool priv_derivation = 0x80000000 & i;
			if (!isPrivate() && priv_derivation) {
				ErrorChecker::ThrowLogicException(Error::Key, "Cannot do private key derivation on public key.");
			}

			HDKeychain child;
			child._valid = false;

			uchar_vector data;
			data += priv_derivation ? _key : _pubkey;
			data.push_back(i >> 24);
			data.push_back((i >> 16) & 0xff);
			data.push_back((i >> 8) & 0xff);
			data.push_back(i & 0xff);

			bytes_t digest = hmac_sha512(_chain_code, data);
			bytes_t left32(digest.begin(), digest.begin() + 32);
			BigInt Il(left32);
			ErrorChecker::CheckLogic(Il >= CURVE_ORDER, Error::Key, "invalid hd keychain");

			if (isPrivate()) {
				BigInt k(_key);
				k += Il;
				k %= CURVE_ORDER;
				ErrorChecker::CheckLogic(k.isZero(), Error::Key, "invalid hd keychain");

				bytes_t child_key = k.getBytes();
				// pad with 0's to make it 33 bytes
				uchar_vector padded_key(33 - child_key.size(), 0);
				padded_key += child_key;
				child._key = padded_key;
				child.updatePubkey();
			} else {
				secp256k1_point K;
				K.bytes(_pubkey);
				K.generator_mul(left32);
				ErrorChecker::CheckLogic(K.is_at_infinity(), Error::Key, "invalid hd keychain");

				child._key = child._pubkey = K.bytes();
			}

			child._version = _version;
			child._depth = _depth + 1;
			child._parent_fp = fp();
			child._child_num = i;
			child._chain_code.assign(digest.begin() + 32, digest.end());

			child._valid = true;
			return child;
		}

		HDKeychain HDKeychain::getChild(const std::string &path) const {
			ErrorChecker::CheckLogic(path.empty(), Error::Key, "invalid hd keychain path");

			std::vector<uint32_t> path_vector;

			size_t i = 0;
			uint64_t n = 0;
			while (i < path.size()) {
				char c = path[i];
				if (c >= '0' && c <= '9') {
					n *= 10;
					n += (uint32_t) (c - '0');
					ErrorChecker::CheckLogic(n >= 0x80000000, Error::Key, "invalid hd keychain path");
					i++;
					if (i >= path.size()) { path_vector.push_back((uint32_t) n); }
				} else if (c == '\'') {
					if (i + 1 < path.size()) {
						if ((i + 2 >= path.size()) || (path[i + 1] != '/') || (path[i + 2] < '0') ||
								(path[i + 2] > '9'))
							ErrorChecker::ThrowLogicException(Error::Key, "invalid hd keychain path");
					}
					n |= 0x80000000;
					path_vector.push_back((uint32_t) n);
					n = 0;
					i += 2;
				} else if (c == '/') {
					if (i + 1 >= path.size() || path[i + 1] < '0' || path[i + 1] > '9')
						ErrorChecker::ThrowLogicException(Error::Key, "invalid hd keychain path");
					path_vector.push_back((uint32_t) n);
					n = 0;
					i++;
				} else {
					ErrorChecker::ThrowLogicException(Error::Key, "invalid hd keychain path");
				}
			}

			HDKeychain child(*this);
			for (auto n: path_vector) {
				child = child.getChild(n);
			}
			return child;
		}

		std::string HDKeychain::toString() const {
			std::stringstream ss;
			ss << "version: " << std::hex << _version << std::endl
				<< "depth: " << depth() << std::endl
				<< "parent_fp: " << _parent_fp << std::endl
				<< "child_num: " << _child_num << std::endl
				<< "chain_code: " << uchar_vector(_chain_code).getHex() << std::endl
				<< "key: " << uchar_vector(_key).getHex() << std::endl
				<< "hash: " << uchar_vector(this->hash()).getHex() << std::endl;
			return ss.str();
		}

		void HDKeychain::updatePubkey() {
			if (isPrivate()) {
				secp256k1_key curvekey;
				curvekey.setPrivKey(bytes_t(_key.begin() + 1, _key.end()));
				_pubkey = curvekey.getPubKey();
			} else {
				_pubkey = _key;
			}
		}

		uint32_t HDKeychain::_priv_version = BITCOIN_HD_PRIVATE_VERSION;
		uint32_t HDKeychain::_pub_version = BITCOIN_HD_PUBLIC_VERSION;

	}

}
