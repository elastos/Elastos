// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PARAMCHECKER_H__
#define __ELASTOS_SDK_PARAMCHECKER_H__

#include "Log.h"
#include "CommonConfig.h"
#include "BigInt.h"

#include <string>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		namespace Error {
			typedef enum {
				InvalidArgument = 20001,
				InvalidPasswd = 20002,
				WrongPasswd = 20003,
				IDNotFound = 20004,
				CreateMasterWalletError = 20005,
				CreateSubWalletError = 20006,
				JsonArrayError = 20007,
				Mnemonic = 20008,
				PubKeyFormat = 20009,
				PubKeyLength = 20010,
				DepositParam = 20011,
				WithdrawParam = 20012,
				CreateTransactionExceedSize = 20013,
				CreateTransaction = 20014,
				Transaction = 20015,
				PathNotExist = 20016,
				PayloadRegisterID = 20017,
				SqliteError = 20018,
				DerivePurpose = 20019,
				WrongAccountType = 20020,
				WrongNetType = 20021,
				InvalidCoinType = 20022,
				NoCurrentMultiSinAccount = 20023,
				MultiSignersCount = 20024,
				MultiSign = 20025,
				KeyStore = 20026,
				LimitGap = 20027,
				Wallet =  20028,
				Key = 20029,
				HexString = 20030,
				SignType = 20031,
				Address = 20032,
				Sign = 20033,
				KeyStoreNeedPhrasePassword = 20034,
				BalanceNotEnough = 20035,
				JsonFormatError = 20036,
				VoteStakeError = 20037,
				GetTransactionInput = 20038,
				InvalidTransaction = 20039,
				GetUnusedAddress = 20040,
				AccountNotSupportVote = 20041,
				WalletNotContainTx = 20042,
				VoteDepositAmountInsufficient = 20043,
				PrivateKeyNotFound = 20044,
				InvalidRedeemScript = 20045,
				AlreadySigned = 20046,
				EncryptError = 20047,
				VerifyError = 20048,
				TxPending = 20049,
				InvalidMnemonicWordCount = 20050,
				InvalidLocalStore = 20051,
				MasterWalletNotExist = 20052,
				InvalidAsset = 20053,
				ReadConfigFileError = 20054,
				InvalidChainID = 20055,
				Other = 29999,
			} Code;
		}

		namespace Exception {
			typedef enum {
				LogicError,
				InvalidArgument,
			} Type;
		}

		class ErrorChecker {
		public:

			static nlohmann::json MakeErrorJson(Error::Code err, const std::string &msg) {
				nlohmann::json j;
				j["Code"] = err;
				j["Message"] = msg;
				return j;
			}

			static nlohmann::json MakeErrorJson(Error::Code err, const std::string &msg, const BigInt &data) {
				nlohmann::json j;
				j["Code"] = err;
				j["Message"] = msg;
				j["Data"] = data.getDec();
				return j;
			}

			static void ThrowParamException(Error::Code err, const std::string &msg) {
				CheckParam(true, err, msg);
			}

			static void ThrowLogicException(Error::Code err, const std::string &msg) {
				CheckLogic(true, err, msg);
			}

			static void CheckParam(bool condition, Error::Code err, const std::string &msg) {
				CheckCondition(condition, err, msg, Exception::Type::InvalidArgument);
			}

			static void CheckLogic(bool condition, Error::Code err, const std::string &msg) {
				CheckCondition(condition, err, msg, Exception::Type::LogicError);
			}

			static void CheckCondition(bool condition, Error::Code err, const std::string &msg,
									   Exception::Type type = Exception::LogicError) {
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

			static void CheckCondition(bool condition, Error::Code err, const std::string &msg, const BigInt &data,
									   Exception::Type type = Exception::LogicError) {
				if (condition) {
					nlohmann::json errJson = MakeErrorJson(err, msg, data);

					if (type == Exception::LogicError) {
						throw std::logic_error(errJson.dump());
					} else if (type == Exception::InvalidArgument) {
						throw std::invalid_argument(errJson.dump());
					}
				}
			}

			static void CheckPassword(const std::string &password, const std::string &msg) {
				CheckCondition(password.size() < MIN_PASSWORD_LENGTH, Error::InvalidPasswd,
							   msg + " password invalid: less than " + std::to_string(MIN_PASSWORD_LENGTH),
							   Exception::InvalidArgument);

				CheckCondition(password.size() > MAX_PASSWORD_LENGTH, Error::InvalidPasswd,
							   msg + " password invalid: more than " + std::to_string(MAX_PASSWORD_LENGTH),
							   Exception::InvalidArgument);
			}

			static void CheckPasswordWithNullLegal(const std::string &password, const std::string &msg) {
				if (password.empty())
					return;

				CheckPassword(password, msg);
			}

			static void CheckParamNotEmpty(const std::string &argument, const std::string &msg) {
				CheckCondition(argument.empty(), Error::InvalidArgument, msg + " should not be empty",
							   Exception::InvalidArgument);
			}

			static void CheckJsonArray(const nlohmann::json &jsonData, size_t count, const std::string &msg) {
				CheckCondition(!jsonData.is_array(), Error::JsonArrayError, msg + " is not json array",
							   Exception::LogicError);
				CheckCondition(jsonData.size() < count, Error::JsonArrayError,
							   msg + " json array size expect at least " + std::to_string(count), Exception::LogicError);
			}

			static void CheckPathExists(const boost::filesystem::path &path) {
				CheckCondition(!boost::filesystem::exists(path), Error::PathNotExist,
							   "Path '" + path.string() + "' do not exist");
			}

			static void CheckPubKeyJsonArray(const nlohmann::json &jsonArray, size_t checkCount,
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

			static void CheckPrivateKey(const std::string &key) {
				// TODO fix here later
				ErrorChecker::CheckCondition(key.find("xprv") != -1, Error::InvalidArgument,
											 "Private key is not support xprv");

				ErrorChecker::CheckCondition(key.length() != 32 * 2, Error::InvalidArgument,
											 "Private key length should be 32 bytes");
			}

		};

	}
}

#endif //__ELASTOS_SDK_PARAMCHECKER_H__
