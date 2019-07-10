// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Log.h"
#include "ErrorChecker.h"

namespace Elastos {
	namespace ElaWallet {

		nlohmann::json ErrorChecker::MakeErrorJson(Error::Code err, const std::string &msg) {
			nlohmann::json j;
			j["Code"] = err;
			j["Message"] = msg;
			return j;
		}

		nlohmann::json ErrorChecker::MakeErrorJson(Error::Code err, const std::string &msg, const BigInt &data) {
			nlohmann::json j;
			j["Code"] = err;
			j["Message"] = msg;
			j["Data"] = data.getDec();
			return j;
		}

		void ErrorChecker::ThrowParamException(Error::Code err, const std::string &msg) {
			CheckParam(true, err, msg);
		}

		void ErrorChecker::ThrowLogicException(Error::Code err, const std::string &msg) {
			CheckLogic(true, err, msg);
		}

		void ErrorChecker::CheckParam(bool condition, Error::Code err, const std::string &msg) {
			CheckCondition(condition, err, msg, Exception::Type::InvalidArgument);
		}

		void ErrorChecker::CheckLogic(bool condition, Error::Code err, const std::string &msg) {
			CheckCondition(condition, err, msg, Exception::Type::LogicError);
		}

		void ErrorChecker::CheckCondition(bool condition, Error::Code err, const std::string &msg,
		                                  Exception::Type type) {
			if (condition) {
				nlohmann::json errJson = MakeErrorJson(err, msg);

				Log::error(errJson.dump());

				if (type == Exception::LogicError) {
					throw std::logic_error(errJson.dump());
				} else if (type == Exception::InvalidArgument) {
					throw std::invalid_argument(errJson.dump());
				}
			}
		}

		void ErrorChecker::CheckCondition(bool condition, Error::Code err, const std::string &msg, const BigInt &data,
		                                  Exception::Type type) {
			if (condition) {
				nlohmann::json errJson = MakeErrorJson(err, msg, data);

				if (type == Exception::LogicError) {
					throw std::logic_error(errJson.dump());
				} else if (type == Exception::InvalidArgument) {
					throw std::invalid_argument(errJson.dump());
				}
			}
		}

		void ErrorChecker::CheckPassword(const std::string &password, const std::string &msg) {
			CheckCondition(password.size() < MIN_PASSWORD_LENGTH, Error::InvalidPasswd,
			               msg + " password invalid: less than " + std::to_string(MIN_PASSWORD_LENGTH),
			               Exception::InvalidArgument);

			CheckCondition(password.size() > MAX_PASSWORD_LENGTH, Error::InvalidPasswd,
			               msg + " password invalid: more than " + std::to_string(MAX_PASSWORD_LENGTH),
			               Exception::InvalidArgument);
		}

		void ErrorChecker::CheckPasswordWithNullLegal(const std::string &password, const std::string &msg) {
			if (password.empty())
				return;

			CheckPassword(password, msg);
		}

		void ErrorChecker::CheckParamNotEmpty(const std::string &argument, const std::string &msg) {
			CheckCondition(argument.empty(), Error::InvalidArgument, msg + " should not be empty",
			               Exception::InvalidArgument);
		}

		void ErrorChecker::CheckJsonArray(const nlohmann::json &jsonData, size_t count, const std::string &msg) {
			CheckCondition(!jsonData.is_array(), Error::JsonArrayError, msg + " is not json array",
			               Exception::LogicError);
			CheckCondition(jsonData.size() < count, Error::JsonArrayError,
			               msg + " json array size expect at least " + std::to_string(count), Exception::LogicError);
		}

		void ErrorChecker::CheckPathExists(const boost::filesystem::path &path) {
			CheckCondition(!boost::filesystem::exists(path), Error::PathNotExist,
			               "Path '" + path.string() + "' do not exist");
		}

		void ErrorChecker::CheckPubKeyJsonArray(const nlohmann::json &jsonArray, size_t checkCount,
		                                        const std::string &msg) {

			CheckJsonArray(jsonArray, checkCount, msg + " pubkey");

			for (nlohmann::json::const_iterator it = jsonArray.begin(); it != jsonArray.end(); ++it) {
				ErrorChecker::CheckParam(!(*it).is_string(), Error::PubKeyFormat, msg + " is not string");

				std::string pubKey = (*it).get<std::string>();

				// TODO fix here later
				ErrorChecker::CheckCondition(pubKey.find("xpub") != -1, Error::PubKeyFormat,
				                             msg + " public key is not support xpub");

				ErrorChecker::CheckCondition(pubKey.length() != 33 * 2 && pubKey.length() != 65 * 2,
				                             Error::PubKeyLength, "Public key length should be 33 or 65 bytes");
				for (nlohmann::json::const_iterator it1 = it + 1; it1 != jsonArray.end(); ++it1) {
					ErrorChecker::CheckParam(pubKey == (*it1).get<std::string>(),
					                         Error::PubKeyFormat, msg + " contain the same");
				}
			}
		}

		void ErrorChecker::CheckPrivateKey(const std::string &key) {
			// TODO fix here later
			ErrorChecker::CheckCondition(key.find("xprv") != -1, Error::InvalidArgument,
			                             "Private key is not support xprv");

			ErrorChecker::CheckCondition(key.length() != 32 * 2, Error::InvalidArgument,
			                             "Private key length should be 32 bytes");
		}

	}
}

