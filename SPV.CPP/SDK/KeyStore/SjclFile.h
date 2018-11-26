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

			const std::string &getIv() const;

			void setIv(const std::string &iv);

			uint32_t getV() const;

			void setV(uint32_t value);

			uint32_t getIter() const;

			void setIter(uint32_t value);

			uint32_t getKs() const;

			void setKs(uint32_t value);

			uint32_t getTs() const;

			void setTs(uint32_t value);

			const std::string &getMode() const;

			void setMode(const std::string &mode);

			const std::string &getAdata() const;

			void setAdata(const std::string &adata);

			const std::string &getCipher() const;

			void setCipher(const std::string &cipher);

			const std::string &getSalt() const;

			void setSalt(const std::string &salt);

			const std::string &getCt() const;

			void setCt(const std::string &ct);

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
