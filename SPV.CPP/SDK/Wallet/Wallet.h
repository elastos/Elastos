// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WALLET_H__
#define __ELASTOS_SDK_WALLET_H__

#include "UTXO.h"

#include <Common/Lockable.h>
#include <Common/ElementSet.h>
#include <Account/SubAccount.h>
#include <Plugin/Transaction/TransactionInput.h>

#include <boost/weak_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <map>
//
//#define TX_FEE_PER_KB        1000ULL     // standard tx fee per kb of tx size, rounded up to nearest kb
//#define TX_OUTPUT_SIZE       34          // estimated size for a typical transaction output
//#define TX_INPUT_SIZE        148         // estimated size for a typical compact pubkey transaction input
//#define TX_MAX_SIZE          (1024 * 1024) // no tx can be larger than this size in bytes
//#define TX_UNCONFIRMED INT32_MAX
//#define DEFAULT_FEE_PER_KB (10000)                  // 10 satoshis-per-byte
//#define MIN_FEE_PER_KB     TX_FEE_PER_KB                       // bitcoind 0.12 default min-relay fee
//#define MAX_FEE_PER_KB     ((TX_FEE_PER_KB*1000100 + 190)/191) // slightly higher than a 10,000bit fee on a 191byte tx
//#define TX_MAX_LOCK_HEIGHT   500000000   // a lockTime below this value is a block height, otherwise a timestamp
//#define TX_MIN_OUTPUT_AMOUNT (TX_FEE_PER_KB*3*(TX_OUTPUT_SIZE + TX_INPUT_SIZE)/1000) //no txout can be below this amount

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		class TransactionOutput;
		class TransactionInput;
		class Address;
		class Asset;
		class IPayload;
		class UTXO;
		class IOutputPayload;
		class DatabaseManager;

		typedef boost::shared_ptr<Asset> AssetPtr;
		typedef boost::shared_ptr<Transaction> TransactionPtr;
		typedef boost::shared_ptr<IPayload> PayloadPtr;
		typedef boost::shared_ptr<UTXO> UTXOPtr;
		typedef std::vector<UTXOPtr> UTXOArray;
		typedef boost::shared_ptr<TransactionOutput> OutputPtr;
		typedef boost::shared_ptr<TransactionInput> InputPtr;
		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;
		typedef boost::shared_ptr<DatabaseManager> DatabaseManagerPtr;

		class Wallet : public Lockable {
		public:
			Wallet(const std::string &walletID,
				   const std::string &chainID,
				   const SubAccountPtr &subAccount,
				   const DatabaseManagerPtr &database);

			virtual ~Wallet();

			void ClearData();

			nlohmann::json GetBasicInfo() const;

			const std::string &GetWalletID() const;

            void GetPublickeys(nlohmann::json &pubkeys, uint32_t index, size_t count, bool internal) const;

            void GetAddresses(AddressArray &addresses, uint32_t index, uint32_t count, bool internal) const;

			void GetCID(AddressArray &cid, uint32_t index, size_t count, bool internal) const;

			AddressPtr GetOwnerDepositAddress() const;

			AddressPtr GetCROwnerDepositAddress() const;

			AddressPtr GetOwnerAddress() const;

			AddressArray GetAllSpecialAddresses() const;

			bytes_t GetOwnerPublilcKey() const;

			bool IsDepositAddress(const Address &addr) const;

			TransactionPtr CreateTransaction(uint8_t type, const PayloadPtr &payload, const UTXOSet &utxo,
                                             const OutputArray &outputs, const std::string &memo, const BigInt &fee,
                                             bool changeBack2FirstInput = false);

			void SignTransaction(const TransactionPtr &tx, const std::string &payPassword) const;

			std::string SignWithAddress(const Address &addr, const std::string &msg, const std::string &payPasswd) const;

			std::string SignDigestWithAddress(const Address &addr, const uint256 &digest, const std::string &payPasswd) const;

			bytes_t SignWithOwnerKey(const bytes_t &msg, const std::string &payPasswd);

		private:
			void LoadUsedAddress();

		protected:
			std::string _walletID, _chainID;

			SubAccountPtr _subAccount;

			DatabaseManagerPtr _database;
		};

		typedef boost::shared_ptr<Wallet> WalletPtr;

	}
}

#endif //__ELASTOS_SDK_WALLET_H__
