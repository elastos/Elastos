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

		const std::string &SjclFile::GetIv() const {
			return _iv;
		}

		void SjclFile::SetIv(const std::string &iv) {
			_iv = iv;
		}

		uint32_t SjclFile::GetV() const {
			return _v;
		}

		void SjclFile::SetV(uint32_t value) {
			_v = value;
		}

		uint32_t SjclFile::GetIter() const {
			return _iter;
		}

		void SjclFile::SetIter(uint32_t value) {
			_iter = value;
		}

		uint32_t SjclFile::GetKs() const {
			return _ks;
		}

		void SjclFile::SetKs(uint32_t value) {
			_ks = value;
		}

		uint32_t SjclFile::GetTs() const {
			return _ts;
		}

		void SjclFile::SetTs(uint32_t value) {
			_ts = value;
		}

		const std::string &SjclFile::GetMode() const {
			return _mode;
		}

		void SjclFile::SetMode(const std::string &mode) {
			_mode = mode;
		}

		const std::string &SjclFile::GetAdata() const {
			return _adata;
		}

		void SjclFile::SetAdata(const std::string &adata) {
			_adata = adata;
		}

		const std::string &SjclFile::GetCipher() const {
			return _cipher;
		}

		void SjclFile::SetCipher(const std::string &cipher) {
			_cipher = cipher;
		}

		const std::string &SjclFile::GetSalt() const {
			return _salt;
		}

		void SjclFile::SetSalt(const std::string &salt) {
			_salt = salt;
		}

		const std::string &SjclFile::GetCt() const {
			return _ct;
		}

		void SjclFile::SetCt(const std::string &ct) {
			_ct = ct;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const SjclFile &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, SjclFile &p) {
			from_json(j, p);

			return j;
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