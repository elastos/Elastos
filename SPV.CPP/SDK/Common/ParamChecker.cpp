// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ParamChecker.h"
#include "CommonConfig.h"

#include <Core/BRBIP39Mnemonic.h>

#include <sstream>
#include <stdexcept>

namespace Elastos {
	namespace ElaWallet {

		nlohmann::json ParamChecker::mkErrorJson(Error::Code err, const std::string &msg) {
			nlohmann::json j;
			j["Code"] = err;
			j["Message"] = msg;
			return j;
		}

		nlohmann::json ParamChecker::mkErrorJson(Error::Code err, const std::string &msg, uint64_t data) {
			nlohmann::json j;
			j["Code"] = err;
			j["Message"] = msg;
			j["Data"] = data;
			return j;
		}

		void
		ParamChecker::checkCondition(bool condition, Error::Code err, const std::string &msg, Exception::Type type) {
			if (condition) {
				nlohmann::json errJson = mkErrorJson(err, msg);

				if (type == Exception::LogicError) {
					throw std::logic_error(errJson.dump());
				} else if (type == Exception::InvalidArgument) {
					throw std::invalid_argument(errJson.dump());
				}
			}
		}

		void
		ParamChecker::checkCondition(bool condition, Error::Code err, const std::string &msg, uint64_t data, Exception::Type type) {
			if (condition) {
				nlohmann::json errJson = mkErrorJson(err, msg, data);

				if (type == Exception::LogicError) {
					throw std::logic_error(errJson.dump());
				} else if (type == Exception::InvalidArgument) {
					throw std::invalid_argument(errJson.dump());
				}
			}
		}

		void ParamChecker::checkPassword(const std::string &password, const std::string &msg) {
			checkCondition(password.size() < MIN_PASSWORD_LENGTH, Error::InvalidPasswd,
						   msg + " password invalid: less than " + std::to_string(MIN_PASSWORD_LENGTH),
						   Exception::LogicError);

			checkCondition(password.size() > MAX_PASSWORD_LENGTH, Error::InvalidPasswd,
						   msg + " password invalid: more than " + std::to_string(MAX_PASSWORD_LENGTH),
						   Exception::LogicError);
		}

		void ParamChecker::checkPasswordWithNullLegal(const std::string &password, const std::string &msg) {
			if (password.empty())
				return;

			checkPassword(password, msg);
		}

		void ParamChecker::checkArgumentNotEmpty(const std::string &argument, const std::string &msg) {
			checkCondition(argument.empty(), Error::InvalidArgument, msg + " is empty", Exception::InvalidArgument);
		}

		void ParamChecker::CheckDecrypt(bool condition) {
			checkCondition(condition, Error::WrongPasswd, "Wrong password");
		}

		void ParamChecker::checkJsonArray(const nlohmann::json &jsonData, size_t count, const std::string &msg) {
			checkCondition(!jsonData.is_array(), Error::JsonArrayError, msg + " is not json array",
						   Exception::LogicError);
			checkCondition(jsonData.size() < count, Error::JsonArrayError,
						   msg + " json array size expect at least " + std::to_string(count), Exception::LogicError);
		}

		void ParamChecker::checkPathExists(const boost::filesystem::path &path) {
			checkCondition(!boost::filesystem::exists(path), Error::PathNotExist,
						   "Path '" + path.string() + "' do not exist");
		}

		void ParamChecker::checkPubKeyJsonArray(const nlohmann::json &jsonArray,
												size_t checkCount, const std::string &msg) {

			checkJsonArray(jsonArray, checkCount, msg + "pubkey");

			std::vector<std::string> signers = jsonArray;

			for (int i = 0; i < signers.size(); i++) {
				ParamChecker::checkCondition(signers[i].find("xpub") != -1, Error::PubKeyFormat,
											 "Public key is not support xpub");
				ParamChecker::checkCondition(signers[i].length() != 33 * 2 && signers[i].length() != 65 * 2,
											 Error::PubKeyLength, "Public key length should be 33 or 65 bytes");
			}
		}

		void ParamChecker::checkPrivateKey(const std::string &key) {
			ParamChecker::checkCondition(key.find("xprv") != -1, Error::InvalidArgument,
										 "Private key is not support xprv");

			ParamChecker::checkCondition(key.length() != 32 * 2, Error::InvalidArgument,
										 "Private key length should be 32 bytes");
		}
	}
}
