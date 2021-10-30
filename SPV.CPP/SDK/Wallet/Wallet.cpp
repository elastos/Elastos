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

			LoadUsedAddress();
		}

		Wallet::~Wallet() {
		}

		void Wallet::ClearData() {
			_database->ClearData();
		}

		TransactionPtr Wallet::CreateTransaction(uint8_t type,
												 const PayloadPtr &payload,
                                                 const UTXOSet &utxo,
												 const OutputArray &outputs,
												 const std::string &memo,
												 const BigInt &fee,
												 bool changeBack2FirstInput) {

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
                tx->AddInput(InputPtr(new TransactionInput((*u)->Hash(), (*u)->Index())));
                if (!_subAccount->GetCode((*u)->GetAddress(), code)) {
                    GetLock().unlock();
                    ErrorChecker::ThrowParamException(Error::Address, "Can't found code and path for input");
                }
                tx->AddUniqueProgram(ProgramPtr(new Program(code, bytes_t())));

                totalInputAmount += (*u)->GetAmount();
            }
            GetLock().unlock();

            if (totalInputAmount < totalOutputAmount + fee) {
                ErrorChecker::ThrowLogicException(Error::BalanceNotEnough, "Available balance is not enough");
            } else if (totalInputAmount > totalOutputAmount + fee) {
                // change
                BigInt changeAmount = totalInputAmount - totalOutputAmount - fee;
                Address changeAddress;
                if (changeBack2FirstInput) {
                    changeAddress = (*utxo.begin())->GetAddress();
                } else {
                    AddressArray addresses;
                    _subAccount->GetAddresses(addresses, 0, 1, false);
                    changeAddress = addresses[0];
                }
                ErrorChecker::CheckParam(!changeAddress.Valid(), Error::Address, "invalid change address");
                tx->AddOutput(OutputPtr(new TransactionOutput(changeAmount, changeAddress)));
            }

            ErrorChecker::CheckLogic(tx->GetOutputs().empty(), Error::InvalidArgument, "outputs empty or input amount not enough");

            tx->SetFee(fee.getUint64());
			if (_chainID == CHAINID_MAINCHAIN)
				tx->SetVersion(Transaction::TxVersion::V09);

			return tx;
		}

        void Wallet::GetPublickeys(nlohmann::json &pubkeys, uint32_t index, size_t count, bool internal) const {
            boost::mutex::scoped_lock scopedLock(lock);
            _subAccount->GetPublickeys(pubkeys, index, count, internal);
        }

        void Wallet::GetAddresses(AddressArray &addresses, uint32_t index, uint32_t count, bool internal) const {
            boost::mutex::scoped_lock scopedLock(lock);
            _subAccount->GetAddresses(addresses, index, count, internal);
		}

		void Wallet::GetCID(AddressArray &cid, uint32_t index, size_t count, bool internal) const {
			boost::mutex::scoped_lock scopedLock(lock);
            _subAccount->GetCID(cid, index, count, false);
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

		std::string Wallet::SignWithAddress(const Address &addr, const std::string &msg, const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithAddress(addr, payPasswd);
			return key.Sign(msg).getHex();
		}

		std::string Wallet::SignDigestWithAddress(const Address &addr, const uint256 &digest, const std::string &payPasswd) const {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->GetKeyWithAddress(addr, payPasswd);
			return key.Sign(digest).getHex();
		}

		bytes_t Wallet::SignWithOwnerKey(const bytes_t &msg, const std::string &payPasswd) {
			boost::mutex::scoped_lock scopedLock(lock);
			Key key = _subAccount->DeriveOwnerKey(payPasswd);
			return key.Sign(msg);
		}

        void Wallet::LoadUsedAddress() {
        }

	}
}