// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PARAMCHECKER_H__
#define __ELASTOS_SDK_PARAMCHECKER_H__

#include <string>
#include <boost/filesystem.hpp>

#include "CMemBlock.h"
#include "nlohmann/json.hpp"

namespace Elastos {
	namespace ElaWallet {

		namespace Error {
			typedef enum {
				InvalidArgument = 20001,
				InvalidPasswd,
				WrongPasswd,
				IDNotFound,
				CreateMasterWalletError,
				CreateSubWalletError,
				JsonArrayError,
				Mnemonic,
				PubKeyFormat,
				PubKeyLength,
				DepositParam,
				WithdrawParam,
				CreateTransaction,
				PathNotExist,
				PayloadRegisterID,
				SqliteError,
				DerivePurpose,
				WrongAccountType,
				WrongNetType,
				InvalidCoinType,
				Transaction,
				NoCurrentMultiSinAccount,
				MultiSignersCount,
				MultiSign,
				KeyStore,
				LimitGap,
				Wallet,
				Key,
				HexString,
				SignType,
				Address,
				Sign,
				Other,
			} Code;
		}

		namespace Exception {
			typedef enum {
				LogicError,
				InvalidArgument,
			} Type;
		}

		class ParamChecker {
		public:

			static nlohmann::json mkErrorJson(Error::Code err, const std::string &msg);

			static void checkCondition(bool condition, Error::Code err, const std::string &msg,
									   Exception::Type type = Exception::LogicError);

			static void checkPassword(const std::string &password, const std::string &msg);

			static void checkPasswordWithNullLegal(const std::string &password, const std::string &msg);

			static void checkArgumentNotEmpty(const std::string &argument, const std::string &msg);

			static void checkDecryptedData(const std::string &data);

			static void checkDecryptedData(const CMBlock &data);

			static void checkJsonArray(const nlohmann::json &jsonData, size_t count, const std::string &msg);

			static void checkPathExists(const boost::filesystem::path &path);

			static void checkPubKeyJsonArray(const nlohmann::json &jsonArray, size_t checkCount,
												const std::string &msg);

			static void checkPrivateKey(const std::string &key);
		};

	}
}

#endif //__ELASTOS_SDK_PARAMCHECKER_H__
