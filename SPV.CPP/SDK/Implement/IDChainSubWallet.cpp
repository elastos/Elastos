// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDChainSubWallet.h"
#include "MasterWallet.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Common/Base64.h>
#include <Common/Utils.h>
#include <WalletCore/CoinInfo.h>
#include <WalletCore/Key.h>
#include <WalletCore/Base58.h>
#include <Plugin/Transaction/Payload/DIDInfo.h>
#include <Plugin/Transaction/Program.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Database/DIDDataStore.h>

#include <set>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace Elastos {
	namespace ElaWallet {

		DIDDetail::DIDDetail() {

		}

		DIDDetail::~DIDDetail() {

		}

		void DIDDetail::SetDIDInfo(const PayloadPtr &didInfo) {
			_didInfo = didInfo;
		}

		const PayloadPtr &DIDDetail::GetDIDInfo() const {
			return _didInfo;
		}

		void DIDDetail::SetBlockHeighht(uint32_t blockHeight) {
			_blockHeight = blockHeight;
		}

		uint32_t DIDDetail::GetBlockHeight() const {
			return _blockHeight;
		}

		void DIDDetail::SetIssuanceTime(time_t issuanceTime) {
			_issuanceTime = issuanceTime;
		}

		time_t DIDDetail::GetIssuanceTime() const {
			return _issuanceTime;
		}

		void DIDDetail::SetTxHash(const std::string &txHash) {
			_txHash = txHash;
		}

		const std::string &DIDDetail::GetTxHash() const {
			return _txHash;
		}

		uint32_t DIDDetail::GetConfirms(uint32_t walletBlockHeight) const {
			if (_blockHeight == TX_UNCONFIRMED)
				return 0;

			return walletBlockHeight >= _blockHeight ? walletBlockHeight - _blockHeight + 1 : 0;
		}

		IDChainSubWallet::IDChainSubWallet(const CoinInfoPtr &info,
		                                   const ChainConfigPtr &config,
		                                   MasterWallet *parent) :
				SidechainSubWallet(info, config, parent) {

			InitDIDList();
		}

		void IDChainSubWallet::InitDIDList() {
			_didList.clear();
			std::vector<DIDEntity> list = _walletManager->loadDIDList();
			size_t len = list.size();
			for (size_t i = 0; i < len; ++i) {
				PayloadPtr infoPtr(new DIDInfo());
				ByteStream stream(list[i].PayloadInfo);
				infoPtr->Deserialize(stream, 0);

				DIDDetailPtr didDetailPtr(new DIDDetail());
				didDetailPtr->SetDIDInfo(infoPtr);
				didDetailPtr->SetBlockHeighht(list[i].BlockHeight);
				didDetailPtr->SetIssuanceTime(list[i].TimeStamp);
				didDetailPtr->SetTxHash(list[i].TxHash);

				Lock();
				InsertDID(didDetailPtr);
				Unlock();
			}
		}

		IDChainSubWallet::~IDChainSubWallet() {

		}

		nlohmann::json
		IDChainSubWallet::CreateIDTransaction(const nlohmann::json &payloadJson, const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);

			Address receiveAddr;
			PayloadPtr payload = nullptr;
			try {
				payload = PayloadPtr(new DIDInfo());
				payload->FromJson(payloadJson, 0);

				DIDInfo *didInfo = static_cast<DIDInfo *>(payload.get());
				ErrorChecker::CheckParam(!didInfo->IsValid(), Error::InvalidArgument, "verify did signature failed");
				std::string id = didInfo->DIDPayload().ID();
				std::vector<std::string> idSplited;
				boost::algorithm::split(idSplited, id, boost::is_any_of(":"), boost::token_compress_on);
				ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument,
										 "invalid id format in payload JSON");
				receiveAddr = Address(idSplited[2]);
				ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::InvalidArgument,
										 "invalid receive addr(id) in payload JSON");
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Create id tx param error: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(0, receiveAddr, Asset::GetELAAssetID())));

			TransactionPtr tx = CreateTx(IDTransaction::didTransaction, payload, "", outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		VerifiableCredential IDChainSubWallet::GetSelfProclaimedCredential(const std::string &didName) const {
			ErrorChecker::CheckParam(didName.empty(), Error::InvalidArgument, "invalid didName");

			VerifiableCredential selfProclaimed;

			CredentialSubject selfProclaimedSubject;
			selfProclaimedSubject.SetDIDName(didName);

			selfProclaimed.SetCredentialSubject(selfProclaimedSubject);

			std::vector<std::string> types = GetVerifiableCredentialTypes(selfProclaimedSubject);
			selfProclaimed.SetTypes(types);

			std::stringstream issuerDate;
			time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm dateTm;
			issuerDate << std::put_time(localtime_r(&t, &dateTm), "%FT%TZ");
			selfProclaimed.SetIssuerDate(issuerDate.str());

			return selfProclaimed;
		}

		VerifiableCredential IDChainSubWallet::GetPersonalInfoCredential(const nlohmann::json &didInfo) const {

			nlohmann::json credentialSubject = didInfo["credentialSubject"];
			ErrorChecker::CheckParam(!credentialSubject.is_object(), Error::InvalidArgument,
			                         "invalid credentialSubject JSON");

			VerifiableCredential verifiableCredential;

			CredentialSubject subject;
			try {
				if (credentialSubject.find("id") != credentialSubject.end()) {
					std::string cid = credentialSubject["id"].get<std::string>();
					if (!cid.empty() && cid.compare(0, sizeof(PREFIX_DID) - 1, PREFIX_DID) != 0) {
						credentialSubject["id"] = PREFIX_DID + cid;
					}
				}
				subject.FromJson(credentialSubject, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "CredentialSubject format err: " + std::string(e.what()));
			}

			verifiableCredential.SetCredentialSubject(subject);

			std::vector<std::string> types = GetVerifiableCredentialTypes(subject);
			verifiableCredential.SetTypes(types);

			std::stringstream issuerDate;
			time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			struct tm dateTm;
			issuerDate << std::put_time(localtime_r(&t, &dateTm), "%FT%TZ");
			verifiableCredential.SetIssuerDate(issuerDate.str());

			return verifiableCredential;
		}

		std::vector<std::string>
		IDChainSubWallet::GetVerifiableCredentialTypes(const CredentialSubject &subject) const {
			std::vector<std::string> types;

			if (subject.GetDIDName().size() > 0) {
				types.push_back("SelfProclaimedCredential");
			}

			if (subject.GetName().size() > 0) {
				types.push_back("BasicProfileCredential");
			}

			if (subject.ID().find(PREFIX_DID) != std::string::npos) {
				types.push_back("ElastosIDteriaCredential");
			}

			if (subject.GetPhone().size() > 0) {
				types.push_back("PhoneCredential");
			}

			if (subject.GetAlipay().size() > 0 || subject.GetWechat().size() > 0 || subject.GetWeibo().size() > 0
			    || subject.GetTwitter().size() > 0 || subject.GetFacebook().size() > 0
			    || subject.GetMicrosoftPassport().size() > 0 || subject.GetGoogleAccount().size() > 0
			    || subject.GetHomePage().size() > 0 || subject.GetEmail().size() > 0) {
				types.push_back("InternetAccountCredential");
			}
			return types;
		}

		nlohmann::json IDChainSubWallet::GetAllDID(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			std::vector<Address> did;
			size_t maxCount = _walletManager->GetWallet()->GetAllDID(did, start, count);

			nlohmann::json didJson;
			for (size_t i = 0; i < did.size(); ++i) {
				didJson.push_back(did[i].String());
			}

			j["DID"] = didJson;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string IDChainSubWallet::Sign(const std::string &did, const std::string &message,
		                                   const std::string &payPassword) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("did: {}", did);
			ArgInfo("message: {}", message);
			ArgInfo("payPasswd: *");

			std::string signature = _walletManager->GetWallet()->SignWithDID(Address(did), message, payPassword);

			ArgInfo("r => {}", signature);

			return signature;
		}

		std::string IDChainSubWallet::SignDigest(const std::string &did, const std::string &digest,
		                                         const std::string &payPassword) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("did: {}", did);
			ArgInfo("digest: {}", digest);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckParam(digest.size() != 64, Error::InvalidArgument, "invalid digest");
			std::string signature = _walletManager->GetWallet()->SignDigestWithDID(Address(did), uint256(digest), payPassword);

			ArgInfo("r => {}", signature);

			return signature;
		}


		bool IDChainSubWallet::VerifySignature(const std::string &publicKey, const std::string &message,
											   const std::string &signature) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey: {}", publicKey);
			ArgInfo("message: {}", message);
			ArgInfo("signature: {}", signature);

			bytes_t pubkey(publicKey), sign(signature);
			Key key(pubkey);
			bool r = key.Verify(message, sign);

			ArgInfo("r => {}", r);
			return r;
		}

		std::string IDChainSubWallet::GetPublicKeyDID(const std::string &pubkey) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey:{}", pubkey);

			ErrorChecker::CheckParamNotEmpty(pubkey, "public key");

			std::string did = Address(PrefixIDChain, pubkey).String();

			ArgInfo("r => {}", did);
			return did;
		}

		nlohmann::json
		IDChainSubWallet::GenerateDIDInfoPayload(const nlohmann::json &didInfo, const std::string &payPasswd) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("didInfo: {}", didInfo.dump());
			ArgInfo("paypassword: *");

			const std::string verificationMethod = "#primary";
			std::string signDID = "";

			ErrorChecker::CheckParam(didInfo.empty() || !didInfo.is_object(), Error::InvalidArgument,
									 "invalid credentialSubject JSON");

			std::string did = didInfo["id"].get<std::string>();
			Address address(did);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "id is invalid");
			std::string id = PREFIX_DID + did;

			std::string didName = didInfo["didName"].get<std::string>();
			ErrorChecker::CheckParam(didName.empty(), Error::InvalidArgument, "invalid didName");

			std::string operation = didInfo["operation"].get<std::string>();
			ErrorChecker::CheckParam(operation != "create" && operation != "update" && operation != "deactivate",
									 Error::InvalidArgument, "invalid operation");

			nlohmann::json pubKeyInfoArray = didInfo["publicKey"];
			ErrorChecker::CheckJsonArray(pubKeyInfoArray, 1, "pubKeyInfoArray");

			time_t expiresTimeStamp = didInfo["expires"].get<uint64_t>();
			std::stringstream expirationDate;
			struct tm dateTm;
			expirationDate << std::put_time(localtime_r(&expiresTimeStamp, &dateTm), "%FT%TZ");
			ErrorChecker::CheckInternetDate(expirationDate.str());

			DIDHeaderInfo headerInfo("elastos/did/1.0", operation);

			DIDPubKeyInfoArray didPubKeyInfoArray;
			for (nlohmann::json::iterator it = pubKeyInfoArray.begin(); it != pubKeyInfoArray.end(); ++it) {
				DIDPubKeyInfo pubKeyInfo;
				bytes_t pubkey;
				if ((*it).find("publicKey") != (*it).end()) {
					std::string pbk = (*it)["publicKey"].get<std::string>();
					pubkey.setHex(pbk);
					(*it)["publicKeyBase58"] = Base58::Encode(pubkey);
				}
				pubKeyInfo.FromJson(*it, 0);
				didPubKeyInfoArray.push_back(pubKeyInfo);

				size_t index = pubKeyInfo.ID().find_last_of("#", pubKeyInfo.ID().size() - 1);
				if (index != std::string::npos) {
					std::string proofUriSegment = pubKeyInfo.ID().substr(index);
					if (proofUriSegment == verificationMethod) {
						signDID = GetPublicKeyDID(pubkey.getHex());
					}
				}
			}

			ErrorChecker::CheckParam(signDID.empty(), Error::InvalidArgument, "publicKey id error");

			VerifiableCredentialArray verifiableCredentials;

			VerifiableCredential selfProclaimed = GetSelfProclaimedCredential(didName);
			selfProclaimed.SetID(id);
			verifiableCredentials.push_back(selfProclaimed);

			if (didInfo.find("credentialSubject") != didInfo.end()) {
				VerifiableCredential verifiableCredential = GetPersonalInfoCredential(didInfo);
				verifiableCredential.SetID(id);
				verifiableCredentials.push_back(verifiableCredential);
			}

			DIDPayloadInfo payloadInfo;
			payloadInfo.SetID(id);
			payloadInfo.SetExpires(expirationDate.str());
			payloadInfo.SetPublickKey(didPubKeyInfoArray);
			payloadInfo.SetVerifiableCredential(verifiableCredentials);

			DIDInfo didInfoPayload;
			didInfoPayload.SetDIDHeader(headerInfo);
			didInfoPayload.SetDIDPlayloadInfo(payloadInfo);

			std::string sourceData = headerInfo.Specification() + headerInfo.Operation() + didInfoPayload.DIDPayloadString();

			std::string signature = Sign(signDID, sourceData, payPasswd);
			DIDProofInfo didProofInfo(verificationMethod, Base64::Encode(signature));
			didInfoPayload.SetDIDProof(didProofInfo);

			nlohmann::json result = didInfoPayload.ToJson(0);
			ArgInfo("r => {}", result.dump());
			return result;
		}

		bool compareDIDInfo(const nlohmann::json &a, const nlohmann::json &b) {
			uint64_t timeStamp1 = a["issuanceDate"].get<uint64_t>();
			uint64_t timeStamp2 = b["issuanceDate"].get<uint64_t>();
			return timeStamp1 > timeStamp2;
		}

		nlohmann::json
		IDChainSubWallet::GetResolveDIDInfo(uint32_t start, uint32_t count, const std::string &did) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("did: {}", did);

			nlohmann::json j;

			bool isDetail = !did.empty();
			if (isDetail) {
				Address address(did);
				ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid did");
			}
			std::string id = PREFIX_DID + did;

			std::map<std::string, DIDDetailPtr> didInfo;
			size_t num = _didList.size();
			size_t pageCount = count;

			for (size_t i = num; i > 0; --i) {
				DIDDetailPtr detailPtr = _didList[i - 1];
				DIDInfo *payload = dynamic_cast<DIDInfo *>(detailPtr->GetDIDInfo().get());
				if (payload) {
					if (isDetail && payload->DIDPayload().ID() == id) {
						didInfo[did] = detailPtr;
						break;
					} else if (!isDetail) {
						if (didInfo.find(payload->DIDPayload().ID()) == didInfo.end()) {
							didInfo[payload->DIDPayload().ID()] = detailPtr;
						}
					}
				}
			}

			if (start >= didInfo.size()) {
				j["DID"] = {};
				j["MaxCount"] = didInfo.size();

				ArgInfo("r => {}", j.dump());
				return j;
			}

			if (didInfo.size() < start + count)
				pageCount = didInfo.size() - start;

			std::vector<nlohmann::json> jsonList;
			for (std::map<std::string, DIDDetailPtr>::iterator iter = didInfo.begin();
					iter != didInfo.end() && jsonList.size() < pageCount; ++iter) {
				jsonList.push_back(ToDIDInfoJson(iter->second, isDetail));
			}

			std::sort(jsonList.begin(), jsonList.end(), compareDIDInfo);

			j["DID"] = jsonList;
			j["MaxCount"] = didInfo.size();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json IDChainSubWallet::ToDIDInfoJson(const DIDDetailPtr &didDetailPtr, bool isDetail) const {
			nlohmann::json summary;

			DIDInfo *didInfo = dynamic_cast<DIDInfo *>(didDetailPtr->GetDIDInfo().get());
			const DIDHeaderInfo &header = didInfo->DIDHeader();
			const DIDPayloadInfo &payloadInfo = didInfo->DIDPayload();

			summary["operation"] = header.Operation();
			summary["id"] = payloadInfo.ID().substr(sizeof(PREFIX_DID) - 1);
			summary["issuanceDate"] = didDetailPtr->GetIssuanceTime();

			uint32_t lastBlockHeight = _walletManager->GetWallet()->LastBlockHeight();
			summary["status"] = didDetailPtr->GetConfirms(lastBlockHeight) <= 6 ? "Pending" : "Confirmed";

			time_t timeStamp;
			Utils::ParseInternetTime(payloadInfo.Expires(), timeStamp);
			summary["expires"] =  timeStamp;

			if (isDetail) {
				nlohmann::json jPublicKeys;
				const DIDPubKeyInfoArray &publicKeys = payloadInfo.PublicKeyInfo();
				for (size_t i = 0; i < publicKeys.size(); ++i) {
					nlohmann::json pbk;
					pbk["id"]= publicKeys[i].ID();
					bytes_t pubkey = Base58::Decode(publicKeys[i].PublicKeyBase58());
					pbk["publicKey"] = pubkey.getHex();

					if (publicKeys[i].Controller().size() > 0){
						pbk["controller"] = publicKeys[i].Controller().substr(sizeof(PREFIX_DID) - 1);
					}
					jPublicKeys.push_back(pbk);
				}
				summary["publicKey"] = jPublicKeys;
			}

			const VerifiableCredentialArray &verifiableCredentialArray = payloadInfo.GetVerifiableCredential();
			for (size_t i = 0; i < verifiableCredentialArray.size(); ++i) {
				if (!verifiableCredentialArray[i].GetCredentialSubject().GetDIDName().empty()) {
					summary["didName"] = verifiableCredentialArray[i].GetCredentialSubject().GetDIDName();
				} else if (isDetail) {
					nlohmann::json credentialSubject = verifiableCredentialArray[i].GetCredentialSubject().ToJson(0);
					if (credentialSubject.find("id") != credentialSubject.end()) {
						std::string id = credentialSubject["id"].get<std::string>();
						credentialSubject["id"] = id;
					}
					summary["credentialSubject"] = credentialSubject;
				}
			}

			return summary;
		}

		void IDChainSubWallet::InsertDID(const DIDDetailPtr &didDetailPtr) {
			size_t i = _didList.size();

			while (i > 0 && _didList[i - 1]->GetIssuanceTime() - didDetailPtr->GetIssuanceTime() > 0) {
				i--;
			}

			_didList.insert(_didList.begin() + i, didDetailPtr);
		}

		void IDChainSubWallet::onTxAdded(const TransactionPtr &tx) {
			SubWallet::onTxAdded(tx);

			if (tx->GetTransactionType() == IDTransaction::didTransaction) {
				DIDInfo *payload = dynamic_cast<DIDInfo *>(tx->GetPayload());
				if (payload) {
					DIDDetailPtr didDetailPtr(new DIDDetail());
					didDetailPtr->SetDIDInfo(tx->GetPayloadPtr());
					didDetailPtr->SetTxHash(tx->GetHash().GetHex());
					didDetailPtr->SetBlockHeighht(tx->GetBlockHeight());
					didDetailPtr->SetIssuanceTime(tx->GetTimestamp());

					Lock();
					InsertDID(didDetailPtr);
					Unlock();

					DIDEntity didEntity;
					didEntity.DID = payload->DIDPayload().ID();
					didEntity.TxHash = didDetailPtr->GetTxHash();
					didEntity.BlockHeight = didDetailPtr->GetBlockHeight();
					didEntity.TimeStamp = didDetailPtr->GetIssuanceTime();

					ByteStream stream;
					didDetailPtr->GetDIDInfo()->Serialize(stream, 0);
					didEntity.PayloadInfo = stream.GetBytes();

					_walletManager->saveDIDInfo(didEntity);
				}
			}

		}

		void IDChainSubWallet::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp) {
			SubWallet::onTxUpdated(hashes, blockHeight,timeStamp);
			Lock();
			size_t didCount = _didList.size();

			for (size_t i = 0;  i < hashes.size(); ++i) {
				for (size_t j = 0; j < didCount; ++j) {
					DIDDetailPtr detailPtr = _didList[j];
					if (detailPtr->GetTxHash() == hashes[i].GetHex()) {
						detailPtr->SetBlockHeighht(blockHeight);
						detailPtr->SetIssuanceTime(timeStamp);
						break;
					}
				}
			}
			Unlock();

			if (!hashes.empty()) {
				_walletManager->updateDIDInfo(hashes, blockHeight, timeStamp);
			}

		}

		void IDChainSubWallet::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			SubWallet::onTxDeleted(hash, notifyUser, recommendRescan);

			Lock();
			size_t len = _didList.size();
			for (size_t i = 0; i < len; ++i) {
				if (_didList[i]->GetTxHash() == hash.GetHex()) {
					_didList.erase(_didList.begin() + i);
					break;
				}
			}
			Unlock();

			_walletManager->deleteDIDInfo(hash.GetHex());
		}

	}
}
