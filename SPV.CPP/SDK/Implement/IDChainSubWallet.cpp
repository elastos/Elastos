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

		void DIDDetail::SetTxTimeStamp(time_t timeStamp) {
			_txTimeStamp = timeStamp;
		}

		time_t DIDDetail::GetTxTimeStamp() const {
			return _txTimeStamp;
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
		                                   MasterWallet *parent,
		                                   const std::string &netType) :
				SidechainSubWallet(info, config, parent, netType) {

			_walletManager->GetWallet()->GenerateCID();
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
				didDetailPtr->SetIssuanceTime(list[i].CreateTime);
				didDetailPtr->SetTxHash(list[i].TxHash);
				didDetailPtr->SetTxTimeStamp(list[i].TimeStamp);

				Lock();
				InsertDID(didDetailPtr);
				Unlock();
			}
		}

		IDChainSubWallet::~IDChainSubWallet() {

		}

		nlohmann::json
		IDChainSubWallet::CreateIDTransaction(const nlohmann::json &payloadJson, const std::string &memo) {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
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
			AddressPtr fromAddr(new Address());

			TransactionPtr tx = wallet->CreateTransaction(IDTransaction::didTransaction, payload, fromAddr, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		VerifiableCredential IDChainSubWallet::GetSelfProclaimedCredential(const std::string &did, const std::string &didName, const std::string &operation) const {
			ErrorChecker::CheckParam(didName.empty(), Error::InvalidArgument, "invalid didName");

			VerifiableCredential selfProclaimed;

			CredentialSubject selfProclaimedSubject;
			selfProclaimedSubject.AddProperties("didName", didName);

			selfProclaimed.SetCredentialSubject(selfProclaimedSubject);

			std::vector<std::string> types = GetVerifiableCredentialTypes(selfProclaimedSubject);
			selfProclaimed.SetTypes(types);

			std::stringstream issuerDate;
			time_t t = 0;
			if (operation == "create") {
				t = time(NULL);
			} else {
				DIDDetailPtr detailInfo = GetDIDInfo(did);
				t = detailInfo->GetIssuanceTime();
			}

			struct tm dateTm;
			issuerDate << std::put_time(localtime_r(&t, &dateTm), "%FT%TZ");
			selfProclaimed.SetIssuerDate(issuerDate.str());

			return selfProclaimed;
		}

		VerifiableCredential IDChainSubWallet::GetPersonalInfoCredential(const nlohmann::json &didInfo) const {

			nlohmann::json credentialSubject = didInfo["credentialSubject"];
			ErrorChecker::CheckParam(!credentialSubject.is_object(), Error::InvalidArgument,
			                         "invalid credentialSubject JSON");

			std::string did = didInfo["id"].get<std::string>();
			std::string operation = didInfo["operation"].get<std::string>();

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
			time_t t = 0;
			if (operation == "create") {
				t = time(NULL);
			} else {
				DIDDetailPtr detailInfo = GetDIDInfo(did);
				t = detailInfo->GetIssuanceTime();
			}

			struct tm dateTm;
			issuerDate << std::put_time(localtime_r(&t, &dateTm), "%FT%TZ");
			verifiableCredential.SetIssuerDate(issuerDate.str());

			return verifiableCredential;
		}

		std::vector<std::string>
		IDChainSubWallet::GetVerifiableCredentialTypes(const CredentialSubject &subject) const {
			std::vector<std::string> types;

			if (subject.HasProperties("didName")) {
				types.push_back("SelfProclaimedCredential");
			}

			if (subject.HasProperties("name")) {
				types.push_back("BasicProfileCredential");
			}

			if (subject.ID().find(PREFIX_DID) != std::string::npos) {
				types.push_back("ElastosIDteriaCredential");
			}

			if (subject.HasProperties("phone")) {
				types.push_back("PhoneCredential");
			}

			if (subject.HasProperties("email") || subject.HasProperties("wechat") || subject.HasProperties("weibo")
			    || subject.HasProperties("twitter") || subject.HasProperties("facebook")
			    || subject.HasProperties("MicrosoftPassport") || subject.HasProperties("googleAccount")
			    || subject.HasProperties("homePage") || subject.HasProperties("taobao")
			    || subject.HasProperties("qq") || subject.HasProperties("telegram") || subject.HasProperties("im")
			    || subject.HasProperties("url")) {
				types.push_back("InternetAccountCredential");
			}
			return types;
		}

		nlohmann::json IDChainSubWallet::GetAllDID(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			AddressArray cid;
			size_t maxCount = _walletManager->GetWallet()->GetAllCID(cid, start, count);

			nlohmann::json didJson;
			for (size_t i = 0; i < cid.size(); ++i) {
				Address tmp(*cid[i]);
				tmp.ConvertToDID();
				didJson.push_back(tmp.String());
			}

			j["DID"] = didJson;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json IDChainSubWallet::GetAllCID(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			AddressArray cid;
			size_t maxCount = _walletManager->GetWallet()->GetAllCID(cid, start, count);

			nlohmann::json cidJosn;
			for (AddressPtr &a : cid) {
				cidJosn.push_back(a->String());
			}

			j["CID"] = cidJosn;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		std::string IDChainSubWallet::Sign(const std::string &DIDOrCID, const std::string &message,
		                                   const std::string &payPassword) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("DIDOrCID: {}", DIDOrCID);
			ArgInfo("message: {}", message);
			ArgInfo("payPasswd: *");

			AddressPtr didAddress(new Address(DIDOrCID));
			std::string signature = _walletManager->GetWallet()->SignWithDID(didAddress, message, payPassword);

			ArgInfo("r => {}", signature);

			return signature;
		}

		std::string IDChainSubWallet::SignDigest(const std::string &DIDOrCID, const std::string &digest,
		                                         const std::string &payPassword) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("DIDOrCID: {}", DIDOrCID);
			ArgInfo("digest: {}", digest);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckParam(digest.size() != 64, Error::InvalidArgument, "invalid digest");
			AddressPtr didAddress(new Address(DIDOrCID));
			std::string signature = _walletManager->GetWallet()->SignDigestWithDID(didAddress, uint256(digest), payPassword);

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

			std::string did = Address(PrefixIDChain, bytes_t(pubkey), true).String();

			ArgInfo("r => {}", did);
			return did;
		}

		std::string IDChainSubWallet::GetPublicKeyCID(const std::string &pubkey) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey: {}", pubkey);

			ErrorChecker::CheckParamNotEmpty(pubkey, "public key");

			std::string cid = Address(PrefixIDChain, bytes_t(pubkey)).String();

			ArgInfo("r => {}", cid);
			return cid;
		}

		nlohmann::json
		IDChainSubWallet::GenerateDIDInfoPayload(const nlohmann::json &didInfo, const std::string &payPasswd) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("didInfo: {}", didInfo.dump());
			ArgInfo("paypassword: *");

			const std::string verificationMethod = PRIMARY_KEY;
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

			std::string previousTxid = "";
			if (operation == UPDATE_DID) {
				DIDDetailPtr detailPtr = GetDIDInfo(did);
				previousTxid = detailPtr->GetTxHash();
			}

			nlohmann::json pubKeyInfoArray = didInfo["publicKey"];
			ErrorChecker::CheckJsonArray(pubKeyInfoArray, 1, "pubKeyInfoArray");

			time_t expiresTimeStamp = didInfo["expires"].get<uint64_t>();
			std::stringstream expirationDate;
			struct tm dateTm;
			expirationDate << std::put_time(localtime_r(&expiresTimeStamp, &dateTm), "%FT%TZ");
			ErrorChecker::CheckInternetDate(expirationDate.str());

			DIDHeaderInfo headerInfo("elastos/did/1.0", operation, previousTxid);

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
				pubKeyInfo.AutoFill(id);
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

			VerifiableCredential selfProclaimed = GetSelfProclaimedCredential(did, didName, operation);
			selfProclaimed.SetID(id);
			selfProclaimed.AutoFill(id);
			verifiableCredentials.push_back(selfProclaimed);

			if (didInfo.find("credentialSubject") != didInfo.end()) {
				VerifiableCredential verifiableCredential = GetPersonalInfoCredential(didInfo);
				verifiableCredential.SetID(id);
				verifiableCredential.AutoFill(id);
				verifiableCredentials.push_back(verifiableCredential);
			}

			DIDPayloadInfo payloadInfo;
			payloadInfo.SetID(id);
			payloadInfo.SetExpires(expirationDate.str());
			payloadInfo.SetPublickKey(didPubKeyInfoArray);
			payloadInfo.SetVerifiableCredential(verifiableCredentials);

			std::string orderedJson = payloadInfo.ToOrderedJson();
			std::string signature = Sign(signDID, orderedJson, payPasswd);
			DIDPayloadProof proof;
			proof.SetSignature(Base64::EncodeURL(signature));
			payloadInfo.SetProof(proof);

			DIDInfo didInfoPayload;
			didInfoPayload.SetDIDHeader(headerInfo);
			didInfoPayload.SetDIDPlayloadInfo(payloadInfo);

			std::string sourceData = "";
			if (headerInfo.Operation() == UPDATE_DID) {
				sourceData = headerInfo.Specification() + headerInfo.Operation() + headerInfo.PreviousTxid() + didInfoPayload.DIDPayloadString();
			} else {
				sourceData = headerInfo.Specification() + headerInfo.Operation() + didInfoPayload.DIDPayloadString();
			}


			signature = Sign(signDID, sourceData, payPasswd);
			DIDProofInfo didProofInfo(verificationMethod, Base64::EncodeURL(signature));
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

		DIDDetailPtr IDChainSubWallet::GetDIDInfo(const std::string &did) const {
			std::string id = PREFIX_DID + did;
			size_t num = _didList.size();
			for (size_t i = num; i > 0; --i) {
				DIDDetailPtr detailPtr = _didList[i - 1];
				DIDInfo *payload = dynamic_cast<DIDInfo *>(detailPtr->GetDIDInfo().get());
				if (payload) {
					if (payload->DIDPayload().ID() == id) {
						return detailPtr;
					}
				}
			}

			Log::warn("no found did : {}", did);
			return nullptr;
		}

		nlohmann::json IDChainSubWallet::ToDIDInfoJson(const DIDDetailPtr &didDetailPtr, bool isDetail) const {
			nlohmann::json summary;

			DIDInfo *didInfo = dynamic_cast<DIDInfo *>(didDetailPtr->GetDIDInfo().get());
			const DIDHeaderInfo &header = didInfo->DIDHeader();
			const DIDPayloadInfo &payloadInfo = didInfo->DIDPayload();

			summary["operation"] = header.Operation();
			if (header.Operation() == UPDATE_DID) {
				summary["previousTxid"] = header.PreviousTxid();
			}
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
				if (!verifiableCredentialArray[i].GetCredentialSubject().HasProperties("didName")) {
					CredentialSubject subject = verifiableCredentialArray[i].GetCredentialSubject();
					summary["didName"] = subject.GetValue("didName");
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
					if (payload->DIDHeader().Operation() == "create") {
						didDetailPtr->SetIssuanceTime(tx->GetTimestamp());
					} else {
						std::string id = payload->DIDPayload().ID();
						std::vector<std::string> idSplited;
						boost::algorithm::split(idSplited, id, boost::is_any_of(":"), boost::token_compress_on);
						nlohmann::json detailInfo = GetResolveDIDInfo(0, 1, idSplited[2]);
						uint64_t issuanceTime = detailInfo["DID"][0]["issuanceDate"].get<uint64_t>();
						didDetailPtr->SetIssuanceTime(issuanceTime);
					}

					Lock();
					InsertDID(didDetailPtr);
					Unlock();

					DIDEntity didEntity;
					didEntity.DID = payload->DIDPayload().ID();
					didEntity.TxHash = didDetailPtr->GetTxHash();
					didEntity.BlockHeight = didDetailPtr->GetBlockHeight();
					didEntity.TimeStamp = didDetailPtr->GetTxTimeStamp();
					didEntity.CreateTime = didDetailPtr->GetIssuanceTime();

					ByteStream stream;
					didDetailPtr->GetDIDInfo()->Serialize(stream, 0);
					didEntity.PayloadInfo = stream.GetBytes();

					_walletManager->saveDIDInfo(didEntity);
				}
			}

		}

		void IDChainSubWallet::onTxUpdated(const std::vector<TransactionPtr> &txns) {
			SubWallet::onTxUpdated(txns);
			std::vector<uint256> hashes;

			Lock();
			size_t didCount = _didList.size();

			for (size_t i = 0; i < txns.size(); ++i) {
				hashes.push_back(txns[i]->GetHash());
				for (size_t j = 0; j < didCount; ++j) {
					DIDDetailPtr detailPtr = _didList[j];
					if (detailPtr->GetTxHash() == txns[i]->GetHash().GetHex()) {
						detailPtr->SetBlockHeighht(txns[i]->GetBlockHeight());
						detailPtr->SetTxTimeStamp(txns[i]->GetTimestamp());
						break;
					}
				}
			}
			Unlock();

			if (!txns.empty())
				_walletManager->updateDIDInfo(hashes, txns[0]->GetBlockHeight(), txns[0]->GetTimestamp());
		}

		void IDChainSubWallet::onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan) {
			SubWallet::onTxDeleted(tx, recommendRescan, false);

			Lock();
			size_t len = _didList.size();
			for (size_t i = 0; i < len; ++i) {
				if (_didList[i]->GetTxHash() == tx->GetHash().GetHex()) {
					_didList.erase(_didList.begin() + i);
					break;
				}
			}
			Unlock();

			_walletManager->deleteDIDInfo(tx->GetHash().GetHex());
		}

	}
}
