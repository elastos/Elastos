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
			 * Derive id by specified purpose and index.
			 * @param purpose for indicating a subtree to derive a chain of sub keys. Purpose should not be 44, which is reserved HD wallet path purpose.
			 * @param index for generating sub keys sequentially.
			 * @return If success return generated id correspond to the \p key.
			 */
			virtual std::string DeriveIdAndKeyForPurpose(
					uint32_t purpose,
					uint32_t index) = 0;

			/**
			 * Verify a id (address).
			 * @param id to be verified.
			 * @return True if valid, otherwise return false.
			 */
			virtual bool IsIdValid(const std::string &id) = 0;

			/**
			 * Generate a program data by passing payload of id transaction
			 * @param id specify id address
			 * @param message is the content of payload in json format.
			 * @param password is the pay password of the master wallet.
			 * @return If success return program data in json format.
			 */
			virtual nlohmann::json GenerateProgram(
					const std::string &id,
					const std::string &message,
					const std::string &password) = 0;

			/**
			 * Sign message through private key of the id.
			 * @param id specify id address
			 * @param message need to signed, it should not be empty.
			 * @param payPassword use to decrypt the root private key temporarily. Pay password should between 8 and 128, otherwise will throw invalid argument exception.
			 * @return signed data of the message.
			 */
			virtual std::string Sign(
					const std::string &id,
					const std::string &message,
					const std::string &password) = 0;

			/**
			 * Get all generated ids.
			 * @return list of id.
			 */
			virtual std::vector<std::string> GetAllIds() const = 0;

			/**
			 * Get public key of the id.
			 * @param id specify id address
			 * @return public key in hex string format.
			 */
			virtual std::string GetPublicKey(const std::string &id) const = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IDAGENT_H__
