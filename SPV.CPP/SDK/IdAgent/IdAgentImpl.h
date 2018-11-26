// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDAGENTIMPL_H__
#define __ELASTOS_SDK_IDAGENTIMPL_H__

#include "IdItem.h"

#include <SDK/Crypto/Key.h>
#include <SDK/Common/Mstream.h>

#include <map>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class MasterWallet;

		struct IdAgentInfo {
			typedef std::map<std::string, IdItem> IdMap;

			IdMap Ids;

			JSON_SM_LS(IdAgentInfo);
			JSON_SM_RS(IdAgentInfo);
			TO_JSON(IdAgentInfo);
			FROM_JSON(IdAgentInfo);
		};

		class IdAgentImpl {
		public:
			IdAgentImpl(MasterWallet *parentWallet, const IdAgentInfo &info);

			~IdAgentImpl();

			std::string DeriveIdAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index);

			bool IsIdValid(const std::string &id);

			std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			std::string GenerateRedeemScript(const std::string &id, const std::string &password);

			std::vector<std::string> GetAllIds() const;

			const IdAgentInfo &GetIdAgentInfo() const;

			std::string GetPublicKey(const std::string &id);

		private:
			KeyPtr generateKey(const std::string &id, const std::string &password);

			bool findIdByPath(const IdItem &item, std::string &id);

		private:

			IdAgentInfo _info;
			MasterWallet *_parentWallet;
		};

	}
}

#endif //__ELASTOS_SDK_IDAGENTIMPL_H__
