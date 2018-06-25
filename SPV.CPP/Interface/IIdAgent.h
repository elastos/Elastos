// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDAGENT_H__
#define __ELASTOS_SDK_IDAGENT_H__

#include <string>

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IIdAgent {
		public:
			/**
			 * Derive id and key by specified purpose and index.
			 * @param purpose for indicating a subtree to derive a chain of sub keys. Purpose should not be 44, which is reserved HD wallet path purpose.
			 * @param index for generating sub keys sequentially.
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return If success return generated id correspond to the \p key.
			 */
			virtual std::string DeriveIdAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index,
					const std::string &payPassword) = 0;

			/**
			 * Verify a id (address).
			 * @param id to be verified.
			 * @return True if valid, otherwise return false.
			 */
			virtual bool IsIdValid(const std::string &id) = 0;

			virtual nlohmann::json GenerateProgram(
					const std::string &id,
					const std::string &message,
					const std::string &password) = 0;

			virtual std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password) = 0;

			virtual std::vector<std::string> GetAllIds() const = 0;

			virtual std::string GetPublicKey(const std::string &id) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IDAGENT_H__
