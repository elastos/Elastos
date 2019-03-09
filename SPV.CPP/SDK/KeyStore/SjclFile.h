// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SJCLFILE_H__
#define __ELASTOS_SDK_SJCLFILE_H__

#include <SDK/Common/Mstream.h>

#include <nlohmann/json.hpp>
#include <string>

namespace Elastos {
	namespace ElaWallet {

		class SjclFile {
		public:
			SjclFile();

			virtual ~SjclFile();

			const std::string &GetIv() const;

			void SetIv(const std::string &iv);

			uint32_t GetV() const;

			void SetV(uint32_t value);

			uint32_t GetIter() const;

			void SetIter(uint32_t value);

			uint32_t GetKs() const;

			void SetKs(uint32_t value);

			uint32_t GetTs() const;

			void SetTs(uint32_t value);

			const std::string &GetMode() const;

			void SetMode(const std::string &mode);

			const std::string &GetAdata() const;

			void SetAdata(const std::string &adata);

			const std::string &GetCipher() const;

			void SetCipher(const std::string &cipher);

			const std::string &GetSalt() const;

			void SetSalt(const std::string &salt);

			const std::string &GetCt() const;

			void SetCt(const std::string &ct);

		private:
			JSON_SM_LS(SjclFile);
			JSON_SM_RS(SjclFile);
			TO_JSON(SjclFile);
			FROM_JSON(SjclFile);

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
