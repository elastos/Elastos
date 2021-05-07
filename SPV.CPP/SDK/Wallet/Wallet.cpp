// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Wallet.h"
#include "WalletCommon.h"

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
#include <Plugin/Transaction/Attribute.h>
#include <Plugin/Transaction/Program.h>
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
			_subAccount(subAccount),
			_database(database) {

			std::vector<std::string> txHashDPoS, txHashCRC, txHashProposal, txHashDID;

			AddressSet usedAddress = LoadUsedAddress();
			_subAccount->SetUsedAddresses(usedAddress);

			_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			_subAccount->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		Wallet::~Wallet() {
		}

		void Wallet::ClearData() {
			_database.lock()->ClearData();
		}

		TransactionPtr Wallet::CreateTransaction(uint8_t type,
												 const PayloadPtr &payload,
                                                 const UTXOSet &utxo,
												 const OutputArray &outputs,
												 const std::string &memo,
												 const BigInt &fee,
												 bool isVote) {

			std::string memoFixed;
            BigInt totalOutputAmount, totalInputAmount;

            TransactionPtr tx = TransactionPtr(new Transaction(type, payload));
            if (!memo.empty()) {
                memoFixed = "type:text,msg:" + memo;
                tx->AddAttribute(AttributePtr(new Attribute(Attribute::Memo, bytes_t(memoFixed.c_str(), memoFixed.size()))));
            }

            tx->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(std::to_string((std::rand() & 0xFFFFFFFF))))));

            for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o)
                totalOutputAmount += (*o)->Amount();

            if (!outputs.empty())
                tx->SetOutputs(outputs);

            GetLock().lock();
            for (UTXOSet::iterator u = utxo.begin(); u != utxo.end(); ++u) {
                bytes_t code;
                std::string path;
                tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
                if (!_subAccount->GetCodeAndPath((*u)->GetAddress(), code, path)) {
                    GetLock().unlock();
                    ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
                }
                tx->AddUniqueProgram(ProgramPtr(new Program(path, code, bytes_t())));

                totalInputAmount += (*u)->GetAmount();
            }
            GetLock().unlock();

            if (totalInputAmount < totalOutputAmount + fee) {
                ErrorChecker::ThrowLogicException(Error::BalanceNotEnough, "Available balance is not enough");
            } else if (totalInputAmount > totalOutputAmount + fee) {
                // change
                BigInt changeAmount = totalInputAmount - totalOutputAmount - fee;
                Address changeAddress;
                if (isVote) {
                    changeAddress = (*utxo.begin())->GetAddress();
                } else {
                    AddressArray addresses = _subAccount->UnusedAddresses(1, 1);
                    changeAddress = addresses[0];
                }
                tx->AddOutput(OutputPtr(new TransactionOutput(changeAmount, changeAddress)));
            }

            ErrorChecker::CheckLogic(tx->GetOutputs().empty(), Error::InvalidArgument, "outputs empty or input amount not enough");

            tx->SetFee(fee.getUint64());
			if (_chainID == CHAINID_MAINCHAIN)
				tx->SetVersion(Transaction::TxVersion::V09);

			return tx;
		}

		Address Wallet::GetReceiveAddress() const {
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
				result.push_back(Address(PrefixStandard, _subAccount->OwnerPubKey()));
				// Owner deposit address
				result.push_back(Address(PrefixDeposit, _subAccount->OwnerPubKey()));
				// CR Owner deposit address
				result.push_back(Address(PrefixDeposit, _subAccount->DIDPubKey()));
			}

			return result;
		}

		bytes_t Wallet::GetOwnerPublilcKey() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _subAccount->OwnerPubKey();
		}

		bool Wallet::IsDepositAddress(const Address &addr) const {
			boost::mutex::scoped_lock scopedLock(lock);

			if (_subAccount->IsProducerDepositAddress(addr))
				return true;
			return _subAccount->IsCRDepositAddress(addr);
		}

		bool Wallet::ContainsAddress(const Address &address) {
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
		Wallet::SignWithDID(const Address &did, const std::string &msg, const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithDID(did, payPasswd);
			return key.Sign(msg).getHex();
		}

		std::string Wallet::SignDigestWithDID(const Address &did, const uint256 &digest,
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

		void Wallet::usedAddressSaved(const AddressSet &usedAddress, bool replace) {
			if (!_database.expired()) {
				std::vector<std::string> addresses;
				for (const Address &a : usedAddress)
					addresses.push_back(a.String());
				_database.lock()->PutUsedAddresses(addresses, replace);
			}
		}

		AddressSet Wallet::LoadUsedAddress() const {
			if (!_database.expired()) {

				AddressSet usedAddress;
				std::vector<std::string> usedAddr = _database.lock()->GetUsedAddresses();

				for (const std::string &addr : usedAddr)
					usedAddress.insert(Address(addr));

				return usedAddress;
			}

			return {};
		}

	}
}