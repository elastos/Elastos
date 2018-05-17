// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SJCLFILE_H__
#define __ELASTOS_SDK_SJCLFILE_H__

#include <string>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace SDK {

		class SjclFile {
		public:
			SjclFile();
			virtual ~SjclFile();

			const std::string &getSalt() const;

		private:
			std::string _iv;
			int _v;
			uint32_t _iter;
			uint32_t _ks;
			uint32_t _ts;
			std::string _mode;
			std::string _adata;
			std::string _cipher;
			std::string _salt;
			std::string _ct;
		};

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json &j, const SjclFile &p);

		void from_json(const nlohmann::json &j, SjclFile &p);

	}
}

#endif //SPVSDK_SJCLFILE_H
