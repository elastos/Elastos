// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDChainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Payload/DIDInfo.h>
#include <SDK/Plugin/Transaction/Program.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/IDTransaction.h>

#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace Elastos {
	namespace ElaWallet {

		IDChainSubWallet::IDChainSubWallet(const CoinInfoPtr &info,
										   const ChainConfigPtr &config,
										   MasterWallet *parent) :
				SidechainSubWallet(info, config, parent) {

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
				ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument, "invalid id format in payload JSON");
				receiveAddr = Address(idSplited[2]);
				ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::InvalidArgument, "invalid receive addr(id) in payload JSON");
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
										   const std::string &payPassword) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("did: {}", did);
			ArgInfo("message: {}", message);
			ArgInfo("payPasswd: *");

			std::string signature = _walletManager->GetWallet()->SignWithDID(Address(did), message, payPassword);

			ArgInfo("r => {}", signature);

			return signature;
		}

		bool IDChainSubWallet::VerifySignature(const std::string &publicKey, const std::string &message,
											   const std::string &signature) {

			// TODO add later
			return false;
		}

		std::string IDChainSubWallet::GetDIDByPublicKey(std::string &pubkey) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey:{}", pubkey);

			ErrorChecker::CheckParamNotEmpty(pubkey, "public key");

			std::string did = Address(PrefixIDChain, pubkey).String();

			ArgInfo("r => {}  {}", did);
			return did;
		}

	}
}
