// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SjclFile.h"

namespace Elastos {
	namespace ElaWallet {

		SjclFile::SjclFile() {

		}

		SjclFile::~SjclFile() {

		}

		nlohmann::json SjclFile::ToJson() const {
			nlohmann::json j;
			j["iv"] = _iv;
			j["v"] = _v;
			j["iter"] = _iter;
			j["ks"] = _ks;
			j["ts"] = _ts;
			j["mode"] = _mode;
			j["adata"] = _adata;
			j["cipher"] = _cipher;
			j["salt"] = _salt;
			j["ct"] = _ct;
			return j;
		}

		void SjclFile::FromJson(const nlohmann::json &j) {
			_iv = j["iv"].get<std::string>();
			_v = j["v"].get<uint32_t>();
			_iter = j["iter"].get<uint32_t>();
			_ks = j["ks"].get<uint32_t>();
			_ts = j["ts"].get<uint32_t>();
			_mode = j["mode"].get<std::string>();
			_adata = j["adata"].get<std::string>();
			_cipher = j["cipher"].get<std::string>();
			_salt = j["salt"].get<std::string>();
			_ct = j["ct"].get<std::string>();
		}
	}
}