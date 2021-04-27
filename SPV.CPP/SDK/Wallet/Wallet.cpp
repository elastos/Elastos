// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "WalletCommon.h"
#include "GroupedAsset.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Key.h>
#include <WalletCore/Address.h>
#include <WalletCore/HDKeychain.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Payload/RegisterAsset.h>
#include <Wallet/UTXO.h>
#include "Database/DatabaseManager.h"

#include <ISubWallet.h>

#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <cstdlib>

namespace Elastos {
	namespace ElaWallet {

		Wallet::Wallet(const std::string &walletID,
					   const std::string &chainID,
					   const SubAccountPtr &subAccount,
					   const DatabaseManagerPtr &database) :
			_walletID(walletID + ":" + chainID),
			_chainID(chainID),
			_feePerKb(DEFAULT_FEE_PER_KB),
			_subAccount(subAccount),
			_database(database) {

			std::vector<std::string> txHashDPoS, txHashCRC, txHashProposal, txHashDID;

            InstallDefaultAsset();

			AddressSet usedAddress = LoadUsedAddress();
			_subAccount->SetUsedAddresses(usedAddress);

			_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		Wallet::~Wallet() {
		}

		void Wallet::ClearData() {
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				it->second->ClearData();
			}
			_database.lock()->ClearData();
		}

		// only support asset of ELA
		TransactionPtr Wallet::Vote(const VoteContent &voteContent, const std::string &memo, bool max,
		                            VoteContentArray &dropedVotes) {
			return _groupedAssets[Asset::GetELAAssetID()]->Vote(voteContent, memo, max, dropedVotes);
		}

		TransactionPtr Wallet::Consolidate(const std::string &memo, const uint256 &assetID) {
			bool containAsset;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				containAsset = ContainsAsset(assetID);
			}

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->Consolidate(memo);

			return tx;
		}

		TransactionPtr Wallet::CreateRetrieveTransaction(uint8_t type, const PayloadPtr &payload, const BigInt &amount,
														 const AddressPtr &fromAddress, const std::string &memo) {
			std::string memoFixed;

			if (!memo.empty())
				memoFixed = "type:text,msg:" + memo;

			TransactionPtr tx = _groupedAssets[Asset::GetELAAssetID()]->CreateRetrieveDepositTx(type, payload, amount, fromAddress, memoFixed);
			tx->SetVersion(Transaction::TxVersion::V09);

			tx->FixIndex();
			return tx;
		}

		TransactionPtr Wallet::CreateTransaction(uint8_t type,
												 const PayloadPtr &payload,
												 const AddressPtr &fromAddress,
												 const OutputArray &outputs,
												 const std::string &memo,
												 bool max,
												 const BigInt &fee) {
			for (const OutputPtr &output : outputs) {
				ErrorChecker::CheckParam(!output->Addr()->Valid(), Error::CreateTransaction,
										 "invalid receiver address");

				ErrorChecker::CheckParam(output->Amount() < 0, Error::CreateTransaction,
										 "output amount should big than zero");
			}

			std::string memoFixed;

			if (!memo.empty())
				memoFixed = "type:text,msg:" + memo;

			ErrorChecker::CheckParam(!IsAssetUnique(outputs), Error::InvalidAsset, "asset is not unique in outputs");

			uint256 assetID = outputs.front()->AssetID();

			bool containAsset;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				containAsset = ContainsAsset(assetID);
			}

			ErrorChecker::CheckParam(!containAsset, Error::InvalidAsset, "asset not found: " + assetID.GetHex());

			TransactionPtr tx = _groupedAssets[assetID]->CreateTxForOutputs(type, payload, outputs, fromAddress, memoFixed, max, fee);

			if (_chainID == CHAINID_MAINCHAIN) {
				tx->SetVersion(Transaction::TxVersion::V09);
			}

			tx->FixIndex();

			return tx;
		}

		AddressPtr Wallet::GetReceiveAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(1, 0)[0];
		}

		size_t Wallet::GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllAddresses(addr, start, count, internal);
		}

		size_t Wallet::GetAllCID(AddressArray &cid, uint32_t start, size_t count) const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllCID(cid, start, count);
		}

		size_t Wallet::GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
										bool containInternal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetAllPublickeys(pubkeys, start, count, containInternal);
		}

		AddressPtr Wallet::GetOwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return AddressPtr(new Address(PrefixDeposit, _subAccount->OwnerPubKey()));
		}

		AddressPtr Wallet::GetCROwnerDepositAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return AddressPtr(new Address(PrefixDeposit, _subAccount->DIDPubKey()));
		}

		AddressPtr Wallet::GetOwnerAddress() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return AddressPtr(new Address(PrefixStandard, _subAccount->OwnerPubKey()));
		}

		AddressArray Wallet::GetAllSpecialAddresses() const {
			AddressArray result;
			boost::mutex::scoped_lock scopedLock(lock);
			if (_subAccount->Parent()->GetSignType() != Account::MultiSign) {
				// Owner address
				result.push_back(AddressPtr(new Address(PrefixStandard, _subAccount->OwnerPubKey())));
				// Owner deposit address
				result.push_back(AddressPtr(new Address(PrefixDeposit, _subAccount->OwnerPubKey())));
				// CR Owner deposit address
				result.push_back(AddressPtr(new Address(PrefixDeposit, _subAccount->DIDPubKey())));
			}

			return result;
		}

		bytes_t Wallet::GetOwnerPublilcKey() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->OwnerPubKey();
		}

		bool Wallet::IsDepositAddress(const AddressPtr &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);

			if (_subAccount->IsProducerDepositAddress(addr))
				return true;
			return _subAccount->IsCRDepositAddress(addr);
		}

		bool Wallet::ContainsAddress(const AddressPtr &address) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _subAccount->ContainsAddress(address);
		}

		void Wallet::GenerateCID() {
			_subAccount->InitCID();
		}

		nlohmann::json Wallet::GetBasicInfo() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->GetBasicInfo();
		}

		const std::string &Wallet::GetWalletID() const {
			return _walletID;
		}

		void Wallet::SignTransaction(const TransactionPtr &tx, const std::string &payPassword) const {
			boost::mutex::scoped_lock scopedLock(lock);
			_subAccount->SignTransaction(tx, payPassword);
		}

		std::string
		Wallet::SignWithDID(const AddressPtr &did, const std::string &msg, const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithDID(did, payPasswd);
			return key.Sign(msg).getHex();
		}

		std::string Wallet::SignDigestWithDID(const AddressPtr &did, const uint256 &digest,
											  const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithDID(did, payPasswd);
			return key.Sign(digest).getHex();
		}

		bytes_t Wallet::SignWithOwnerKey(const bytes_t &msg, const std::string &payPasswd) {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->DeriveOwnerKey(payPasswd);
			return key.Sign(msg);
		}

		AddressArray Wallet::UnusedAddresses(uint32_t gapLimit, bool internal) {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->UnusedAddresses(gapLimit, internal);
		}

		AssetPtr Wallet::GetAsset(const uint256 &assetID) const {
			boost::mutex::scoped_lock scopedLock(lock);
			if (!ContainsAsset(assetID)) {
				Log::warn("asset not found: {}", assetID.GetHex());
				return nullptr;
			}

			return _groupedAssets[assetID]->GetAsset();
		}

		nlohmann::json Wallet::GetAllAssets() const {
			boost::mutex::scoped_lock scopedLock(lock);
			nlohmann::json j;
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it) {
				j.push_back(it->first.GetHex());
			}
			return j;
		}

		bool Wallet::AssetNameExist(const std::string &name) const {
			boost::mutex::scoped_lock scopedLock(lock);
			for (GroupedAssetMap::iterator it = _groupedAssets.begin(); it != _groupedAssets.end(); ++it)
				if (it->second->GetAsset()->GetName() == name)
					return true;
			return false;
		}

		GroupedAssetPtr Wallet::GetGroupedAsset(const uint256 &assetID) const {
			GroupedAssetMap::iterator it = _groupedAssets.find(assetID);
			if (it != _groupedAssets.end())
				return it->second;

			return nullptr;
		}

		bool Wallet::ContainsAsset(const uint256 &assetID) const {
			return _groupedAssets.find(assetID) != _groupedAssets.end();
		}

		bool Wallet::IsAssetUnique(const OutputArray &outputs) const {
			for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
				if (outputs.front()->AssetID() != (*o)->AssetID())
					return false;
			}

			return true;
		}

		void Wallet::InstallAssets(const std::vector<AssetPtr> &assets) {
			for (size_t i = 0; i < assets.size(); ++i) {
				if (!ContainsAsset(assets[i]->GetHash())) {
					_groupedAssets[assets[i]->GetHash()] = GroupedAssetPtr(new GroupedAsset(this, assets[i]));
				} else {
					Log::debug("asset {} already exist", assets[i]->GetHash().GetHex());
				}
			}
		}

		void Wallet::InstallDefaultAsset() {
			AssetPtr asset(new Asset());
			_groupedAssets[asset->GetHash()] = GroupedAssetPtr(new GroupedAsset(this, asset));
		}

		void Wallet::usedAddressSaved(const AddressSet &usedAddress, bool replace) {
			if (!_database.expired()) {
				std::vector<std::string> addresses;
				for (const AddressPtr &a : usedAddress)
					addresses.push_back(a->String());
				_database.lock()->PutUsedAddresses(addresses, replace);
			}
		}

		AddressSet Wallet::LoadUsedAddress() const {
			if (!_database.expired()) {

				AddressSet usedAddress;
				std::vector<std::string> usedAddr = _database.lock()->GetUsedAddresses();

				for (const std::string &addr : usedAddr)
					usedAddress.insert(AddressPtr(new Address(addr)));

				return usedAddress;
			}

			return {};
		}

	}
}