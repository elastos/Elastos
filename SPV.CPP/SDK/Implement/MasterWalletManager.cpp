// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>
#include <SDK/Common/Utils.h>
#include <Core/BRBase58.h>

#include "MasterWalletManager.h"
#include "Log.h"
#include "MasterWallet.h"
#include "ParamChecker.h"
#include "Config.h"

using namespace boost::filesystem;

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"

namespace Elastos {
	namespace ElaWallet {

		MasterWalletManager::MasterWalletManager(const std::string &rootPath) :
			_rootPath(rootPath),
			_p2pEnable(true) {
			initMasterWallets();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath) :
			_masterWalletMap(walletMap),
			_rootPath(rootPath),
			_p2pEnable(true) {
		}

		MasterWalletManager::~MasterWalletManager() {
			std::vector<std::string> masterWalletIds;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [&masterWalletIds](const MasterWalletMap::value_type &item) {
							  masterWalletIds.push_back(item.first);
						  });
			std::for_each(masterWalletIds.begin(), masterWalletIds.end(), [this](const std::string &id) {
				this->removeWallet(id);
			});
		}

		void MasterWalletManager::SaveConfigs() {
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [](const MasterWalletMap::value_type &item) {
							  MasterWallet *masterWallet = static_cast<MasterWallet *>(item.second);
							  masterWallet->Save();
						  });
		}

		std::string MasterWalletManager::GenerateMnemonic(const std::string &language) {
			return MasterWallet::GenerateMnemonic(language, _rootPath);
		}

		IMasterWallet *MasterWalletManager::CreateMasterWallet(
			const std::string &masterWalletId,
			const std::string &mnemonic,
			const std::string &phrasePassword,
			const std::string &payPassword,
			const std::string &language) {

			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  language, _p2pEnable, _rootPath, CreateNormal);

			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletId,
																		const nlohmann::json &coSigners,
																		uint32_t requiredSignCount) {
			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			ParamChecker::checkPubKeyJsonArray(coSigners, requiredSignCount, "Signers");

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, coSigners, requiredSignCount,
														  _rootPath, _p2pEnable, CreateMultiSign);
			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletId,
																		const std::string &privKey,
																		const std::string &payPassword,
																		const nlohmann::json &coSigners,
																		uint32_t requiredSignCount) {
			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			ParamChecker::checkPubKeyJsonArray(coSigners, requiredSignCount - 1, "Signers");
			ParamChecker::checkPrivateKey(privKey);

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, privKey, payPassword, coSigners,
														  requiredSignCount, _rootPath, _p2pEnable,
														  CreateMultiSign);
			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletId,
																		const std::string &mnemonic,
																		const std::string &phrasePassword,
																		const std::string &payPassword,
																		const nlohmann::json &coSigners,
																		uint32_t requiredSignCount,
																		const std::string &language) {
			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			ParamChecker::checkPubKeyJsonArray(coSigners, requiredSignCount - 1, "Signers");

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  language, coSigners, requiredSignCount, _p2pEnable,
														  _rootPath, CreateMultiSign);

			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			std::vector<IMasterWallet *> result;
			for (MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it) {
				result.push_back(GetWallet(it->first));
			}
			return result;
		};

		void MasterWalletManager::removeWallet(const std::string &masterWalletId, bool saveMaster) {
			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) == _masterWalletMap.end())
				return;

			IMasterWallet *masterWallet = _masterWalletMap[masterWalletId];

			MasterWallet *masterWalletInner = static_cast<MasterWallet *>(masterWallet);
			if (saveMaster) {
				SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] Begin save ({}).", masterWalletId);
				masterWalletInner->Save();
				SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] End save ({}).", masterWalletId);
			} else {
				SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] Begin clear local ({}).",
							 masterWalletId);
				masterWalletInner->ClearLocal();
				SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] End clear local ({}).",
							 masterWalletId);
			}

			SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] Begin destroy sub wallets ({}).",
						 masterWalletId);
			std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
			for (int i = 0; i < subWallets.size(); ++i) {
				masterWallet->DestroyWallet(subWallets[i]);
			}
			SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] End destroy sub wallets of ({}).",
						 masterWalletId);

			SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] Removing master wallet from map ({}).",
						 masterWalletId);
			_masterWalletMap.erase(masterWalletId);

			SPDLOG_DEBUG(Log::getLogger(), "[MasterWalletManager::removeWallet] Deleting master wallet ({}).",
						 masterWalletId);
			delete masterWallet;
		}

		void MasterWalletManager::DestroyWallet(const std::string &masterWalletId) {
			removeWallet(masterWalletId, false);
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithKeystore(const std::string &masterWalletId,
													  const nlohmann::json &keystoreContent,
													  const std::string &backupPassword,
													  const std::string &payPassword,
													  const std::string &phrasePassword) {
			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];


			MasterWallet *masterWallet = new MasterWallet(masterWalletId, keystoreContent, backupPassword,
														  payPassword, phrasePassword, _rootPath, _p2pEnable,
														  ImportFromKeyStore);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  const std::string &language) {
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkArgumentNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  language, _p2pEnable, _rootPath, ImportFromMnemonic);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		nlohmann::json
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword) {

			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			return wallet->exportKeyStore(backupPassword, payPassword);
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) {

			ParamChecker::checkCondition(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ParamChecker::checkPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);

			std::string mnemonic;
			ParamChecker::checkCondition(!wallet->exportMnemonic(payPassword, mnemonic), Error::WrongPasswd,
										 "Wrong pay password");
			return mnemonic;
		}

		nlohmann::json MasterWalletManager::EncodeTransactionToString(const nlohmann::json &tx) {
			Transaction txn;

			txn.fromJson(tx);

			ByteStream stream;
			txn.Serialize(stream);

			CMBlock hex = stream.getBuffer();
			size_t len = BRBase58CheckEncode(NULL, 0, hex, hex.GetSize());
			char buf[len + 1];

			BRBase58CheckEncode(buf, sizeof(buf), hex, hex.GetSize());

			nlohmann::json result;

			result["Algorithm"] = "base58";
			result["Data"] = std::string(buf);

			return result;
		}

		nlohmann::json MasterWalletManager::DecodeTransactionFromString(const nlohmann::json &cipher) {
			Transaction txn;

			std::string algorithm = cipher["Algorithm"].get<std::string>();
			std::string data = cipher["Data"].get<std::string>();

			CMBlock rawHex;
			if (algorithm == "base58") {
				size_t len = BRBase58CheckDecode(NULL, 0, data.c_str());
				ParamChecker::checkCondition(len == 0, Error::Transaction,
											 "Decode tx from base58 error");

				rawHex.Resize(len);
				BRBase58CheckDecode(rawHex, rawHex.GetSize(), data.c_str());
			} else {
				ParamChecker::checkCondition(true, Error::Transaction,
											 "Decode tx with unknown algorithm");
			}

			ByteStream stream;
			stream.writeBytes(rawHex, rawHex.GetSize());
			stream.setPosition(0);
			txn.Deserialize(stream);

			return txn.toJson();
		}

		void MasterWalletManager::initMasterWallets() {
			path rootPath = _rootPath;

			Log::getLogger()->critical("spvsdk version {}", SPVSDK_VERSION_MESSAGE);

			ParamChecker::checkPathExists(rootPath);

			directory_iterator it{rootPath};
			while (it != directory_iterator{}) {

				path temp = *it;
				if (!exists(temp) || !is_directory(temp)) {
					++it;
					continue;
				}

				std::string masterWalletId = temp.filename().string();
				temp /= MASTER_WALLET_STORE_FILE;
				if (exists(temp)) {
					_masterWalletMap[masterWalletId] = nullptr;
				}
				++it;
			}

			if (_masterWalletMap.size() > 0)
				Log::getLogger()->info("{} master wallets were loaded from local store", _masterWalletMap.size());
		}

		std::vector<std::string> MasterWalletManager::GetAllMasterWalletIds() const {
			std::vector<std::string> result;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [&result](const MasterWalletMap::value_type &item) {
							  result.push_back(item.first);
						  });
			return result;
		}

		IMasterWallet *MasterWalletManager::GetWallet(const std::string &masterWalletId) const {
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.cend() &&
				_masterWalletMap[masterWalletId] != nullptr)
				return _masterWalletMap[masterWalletId];

			path masterWalletStoreFile = _rootPath;
			masterWalletStoreFile /= masterWalletId;
			masterWalletStoreFile /= MASTER_WALLET_STORE_FILE;

			MasterWallet *masterWallet = new MasterWallet(masterWalletStoreFile, _rootPath, _p2pEnable,
														  ImportFromLocalStore);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}
	}
}