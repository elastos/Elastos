// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SjclFile.h"

namespace Elastos {
	namespace SDK {

		SjclFile::SjclFile() {

		}

		SjclFile::~SjclFile() {

		}

		const std::string &SjclFile::getIv() const {
			return _iv;
		}

		void SjclFile::setIv(const std::string &iv) {
			_iv = iv;
		}

		uint32_t SjclFile::getV() const {
			return _v;
		}

		void SjclFile::setV(uint32_t value) {
			_v = value;
		}

		uint32_t SjclFile::getIter() const {
			return _iter;
		}

		void SjclFile::setIter(uint32_t value) {
			_iter = value;
		}

		uint32_t SjclFile::getKs() const {
			return _ks;
		}

		void SjclFile::setKs(uint32_t value) {
			_ks = value;
		}

		uint32_t SjclFile::getTs() const {
			return _ts;
		}

		void SjclFile::setTs(uint32_t value) {
			_ts = value;
		}

		const std::string &SjclFile::getMode() const {
			return _mode;
		}

		void SjclFile::setMode(const std::string &mode) {
			_mode = mode;
		}

		const std::string &SjclFile::getAdata() const {
			return _adata;
		}

		void SjclFile::setAdata(const std::string &adata) {
			_adata = adata;
		}

		const std::string &SjclFile::getCipher() const {
			return _cipher;
		}

		void SjclFile::setCipher(const std::string &cipher) {
			_cipher = cipher;
		}

		const std::string &SjclFile::getSalt() const {
			return _salt;
		}

		void SjclFile::setSalt(const std::string &salt) {
			_salt = salt;
		}

		const std::string &SjclFile::getCt() const {
			return _ct;
		}

		void SjclFile::setCt(const std::string &ct) {
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
			j["iv"] = p.getIv();
			j["v"] = p.getV();
			j["iter"] = p.getIter();
			j["ks"] = p.getKs();
			j["ts"] = p.getTs();
			j["mode"] = p.getMode();
			j["adata"] = p.getAdata();
			j["cipher"] = p.getCipher();
			j["salt"] = p.getSalt();
			j["ct"] = p.getCt();
		}

		void from_json(const nlohmann::json &j, SjclFile &p) {
			p.setIv(j["iv"].get<std::string>());
			p.setV(j["v"].get<uint32_t>());
			p.setIter(j["iter"].get<uint32_t>());
			p.setKs(j["ks"].get<uint32_t>());
			p.setTs(j["ts"].get<uint32_t>());
			p.setMode(j["mode"].get<std::string>());
			p.setAdata(j["adata"].get<std::string>());
			p.setCipher(j["cipher"].get<std::string>());
			p.setSalt(j["salt"].get<std::string>());
			p.setCt(j["ct"].get<std::string>());
		}
	}
}