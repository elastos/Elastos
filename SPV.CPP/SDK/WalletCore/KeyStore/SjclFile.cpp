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

		void to_json(nlohmann::json &j, const SjclFile &p) {
			j["iv"] = p.GetIv();
			j["v"] = p.GetV();
			j["iter"] = p.GetIter();
			j["ks"] = p.GetKs();
			j["ts"] = p.GetTs();
			j["mode"] = p.GetMode();
			j["adata"] = p.GetAdata();
			j["cipher"] = p.GetCipher();
			j["salt"] = p.GetSalt();
			j["ct"] = p.GetCt();
		}

		void from_json(const nlohmann::json &j, SjclFile &p) {
			p.SetIv(j["iv"].get<std::string>());
			p.SetV(j["v"].get<uint32_t>());
			p.SetIter(j["iter"].get<uint32_t>());
			p.SetKs(j["ks"].get<uint32_t>());
			p.SetTs(j["ts"].get<uint32_t>());
			p.SetMode(j["mode"].get<std::string>());
			p.SetAdata(j["adata"].get<std::string>());
			p.SetCipher(j["cipher"].get<std::string>());
			p.SetSalt(j["salt"].get<std::string>());
			p.SetCt(j["ct"].get<std::string>());
		}
	}
}