// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDAGENTIMPL_H__
#define __ELASTOS_SDK_IDAGENTIMPL_H__

#include "IDItem.h"

#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/Common/Mstream.h>

#include <map>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class MasterWallet;

		struct IDAgentInfo {
			typedef std::map<std::string, IDItem> IDMap;

			IDMap IDs;

			JSON_SM_LS(IDAgentInfo);
			JSON_SM_RS(IDAgentInfo);
			TO_JSON(IDAgentInfo);
			FROM_JSON(IDAgentInfo);
		};

		class IDAgentImpl {
		public:
			IDAgentImpl(MasterWallet *parentWallet);

			~IDAgentImpl();

			Address DeriveIDAndKeyForPurpose(
				uint32_t purpose,
				uint32_t index);

			bool IsIDValid(const std::string &id);

			bytes_t Sign(const std::string &id, const bytes_t &data, const std::string &passwd);

			std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			std::string GenerateRedeemScript(const std::string &id, const std::string &password);

			std::vector<std::string> GetAllIDs() const;

			const IDAgentInfo &GetIDAgentInfo() const;

			bytes_t GetPublicKey(const std::string &id) const;

		private:
			KeyPtr generateKey(const std::string &id, const std::string &password);

			bool findIDByPath(const IDItem &item, std::string &id);

		private:

			IDAgentInfo _info;
			MasterWallet *_parentWallet;
		};

	}
}

#endif //__ELASTOS_SDK_IDAGENTIMPL_H__
