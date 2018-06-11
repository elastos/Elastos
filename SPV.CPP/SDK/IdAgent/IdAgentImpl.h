// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDAGENTIMPL_H__
#define __ELASTOS_SDK_IDAGENTIMPL_H__

#include <map>
#include <vector>

#include "IdPath.h"
#include "Wrapper/Key.h"

namespace Elastos {
	namespace SDK {

		class MasterWallet;

		class IdAgentImpl {
		public:
			IdAgentImpl(MasterWallet *parentWallet);

			~IdAgentImpl();

			std::string DeriveIdAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index,
					const std::string &payPassword);

			bool IsIdValid(const std::string &id);

			std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password);

			std::string GenerateRedeemScript(const std::string &id, const std::string &password);

			std::vector<std::string> GetAllIds() const;

		private:
			KeyPtr generateKey(const std::string &id, const std::string &password);

			bool findIdByPath(const IdPath &path, std::string &id);

		private:
			typedef std::map<std::string, IdPath> IdMap;
			IdMap _idMap;

			MasterWallet *_parentWallet;
		};

	}
}

#endif //__ELASTOS_SDK_IDAGENTIMPL_H__
