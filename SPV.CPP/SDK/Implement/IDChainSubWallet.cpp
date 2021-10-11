/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "IDChainSubWallet.h"
#include "MasterWallet.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <WalletCore/CoinInfo.h>
#include <WalletCore/Key.h>
#include <Plugin/Transaction/Payload/DIDInfo.h>
#include <Plugin/Transaction/Program.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/IDTransaction.h>

#include <set>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace Elastos {
	namespace ElaWallet {

		IDChainSubWallet::IDChainSubWallet(const CoinInfoPtr &info,
		                                   const ChainConfigPtr &config,
		                                   MasterWallet *parent,
		                                   const std::string &netType) :
				SidechainSubWallet(info, config, parent, netType) {
		}

		IDChainSubWallet::~IDChainSubWallet() {

		}

		nlohmann::json IDChainSubWallet::CreateIDTransaction(
		        const nlohmann::json &inputsJson,
		        const nlohmann::json &payloadJson,
		        const std::string &memo,
		        const std::string &fee) {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);
			ArgInfo("fee: {}", fee);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

            BigInt userFee;
            userFee.setDec(fee);
            ErrorChecker::CheckParam(userFee < 0, Error::InvalidArgument, "invalid fee");

            Address receiveAddr;
			PayloadPtr payload = nullptr;
            std::vector<OutputPtr> outputs;
			try {
				payload = PayloadPtr(new DIDInfo());
				payload->FromJson(payloadJson, 0);

				DIDInfo *didInfo = static_cast<DIDInfo *>(payload.get());
//				ErrorChecker::CheckParam(!didInfo->IsValid(0), Error::InvalidArgument, "verify did signature failed");
                std::vector<std::string> idSplited;
                if (!didInfo->DIDPayload().Controller().empty()) {
                    for (const std::string &controller : didInfo->DIDPayload().Controller()) {
                        boost::algorithm::split(idSplited, controller, boost::is_any_of(":"), boost::token_compress_on);
                        ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument,
                                                 "invalid id format in payload JSON");
                        receiveAddr = Address(idSplited[2]);
                        ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::InvalidArgument,
                                                 "invalid receive addr(id) in payload JSON");
                        outputs.push_back(OutputPtr(new TransactionOutput(0, receiveAddr, Asset::GetELAAssetID())));
                    }
                } else {
                    std::string id = didInfo->DIDPayload().ID();
                    boost::algorithm::split(idSplited, id, boost::is_any_of(":"), boost::token_compress_on);
                    ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument,
                                             "invalid id format in payload JSON");
                    receiveAddr = Address(idSplited[2]);
                    ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::InvalidArgument,
                                             "invalid receive addr(id) in payload JSON");
                    outputs.push_back(OutputPtr(new TransactionOutput(0, receiveAddr, Asset::GetELAAssetID())));
                }
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Create id tx param error: " + std::string(e.what()));
			}

			TransactionPtr tx = wallet->CreateTransaction(IDTransaction::didTransaction, payload, utxo, outputs, memo, userFee);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		nlohmann::json IDChainSubWallet::GetDID(uint32_t index, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("index: {}", index);
			ArgInfo("count: {}", count);
			ArgInfo("internal: {}", internal);

			nlohmann::json j;
			AddressArray cid;
            _walletManager->GetWallet()->GetCID(cid, index, count, internal);

			nlohmann::json didJson;
			for (size_t i = 0; i < cid.size(); ++i) {
				Address tmp = cid[i];
				tmp.ConvertToDID();
				didJson.push_back(tmp.String());
			}

			j["DID"] = didJson;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json IDChainSubWallet::GetCID(uint32_t index, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("index: {}", index);
			ArgInfo("count: {}", count);
			ArgInfo("internal: {}", internal);

			nlohmann::json j;
			AddressArray cid;
            _walletManager->GetWallet()->GetCID(cid, index, count, internal);

			nlohmann::json cidJosn;
			for (Address &a : cid) {
				cidJosn.push_back(a.String());
			}

			j["CID"] = cidJosn;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		std::string IDChainSubWallet::Sign(const std::string &DIDOrCID, const std::string &message,
		                                   const std::string &payPassword) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("DIDOrCID: {}", DIDOrCID);
			ArgInfo("message: {}", message);
			ArgInfo("payPasswd: *");

			Address didAddress(DIDOrCID);
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
			Address didAddress(DIDOrCID);
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
			Key key(CTElastos, pubkey);
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

	}
}
