// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELANEWWALLETJSON_H__
#define __ELASTOS_SDK_ELANEWWALLETJSON_H__

#include "ElaWebWalletJson.h"

namespace Elastos {
	namespace SDK {

		class ElaNewWalletJson :
				public ElaWebWalletJson {
		public:
			ElaNewWalletJson();

			~ElaNewWalletJson();

			const std::string &getID() const;

			void setID(const std::string &id);

			const std::string &getIDInfo() const;

			void setIDInfo(const std::string &value);

			uint32_t getEarliestPeerTime() const;

			void setEaliestPeerTime(uint32_t time);

			const std::string &getMnemonicLanguage() const;

			void setMnemonicLanguage(const std::string &language);

		private:
			std::string _id;
			std::string _idInfo;
			std::string _mnemonicLanguage;
			uint32_t _earliestPeerTime;
		};

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json &j, const ElaNewWalletJson &p);

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p);

	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
