// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SJCLFILE_H__
#define __ELASTOS_SDK_SJCLFILE_H__

#include <Common/JsonSerializer.h>

#include <nlohmann/json.hpp>
#include <string>

namespace Elastos {
	namespace ElaWallet {

		class SjclFile : public JsonSerializer {
		public:
			SjclFile();

			virtual ~SjclFile();

			const std::string &GetIv() const { return _iv; }

			void SetIv(const std::string &iv) { _iv = iv; }

			uint32_t GetV() const { return _v; }

			void SetV(uint32_t value) { _v = value; }

			uint32_t GetIter() const { return _iter; }

			void SetIter(uint32_t value) { _iter = value; }

			uint32_t GetKs() const { return _ks; }

			void SetKs(uint32_t value) { _ks = value; }

			uint32_t GetTs() const { return _ts; }

			void SetTs(uint32_t value) { _ts = value; }

			const std::string &GetMode() const { return _mode; }

			void SetMode(const std::string &mode) { _mode = mode; }

			const std::string &GetAdata() const { return _adata; }

			void SetAdata(const std::string &adata) { _adata = adata; }

			const std::string &GetCipher() const { return _cipher; }

			void SetCipher(const std::string &cipher) { _cipher = cipher; }

			const std::string &GetSalt() const { return _salt; }

			void SetSalt(const std::string &salt) { _salt = salt; }

			const std::string &GetCt() const { return _ct; }

			void SetCt(const std::string &ct) { _ct = ct; }

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

		private:
			std::string _iv;
			uint32_t _v;
			uint32_t _iter;
			uint32_t _ks;
			uint32_t _ts;
			std::string _mode;
			std::string _adata;
			std::string _cipher;
			std::string _salt;
			std::string _ct;
		};
	}
}

#endif //SPVSDK_SJCLFILE_H
