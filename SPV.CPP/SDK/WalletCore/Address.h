// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ADDRESS_H__
#define __ELASTOS_SDK_ADDRESS_H__

#include <Common/typedefs.h>
#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

#define ELA_SIDECHAIN_DESTROY_ADDR "1111111111111111111114oLvT2"
#define OP_0           0x00
#define OP_PUSHDATA1   0x4c
#define OP_PUSHDATA2   0x4d
#define OP_PUSHDATA4   0x4e
#define OP_1NEGATE     0x4f
#define OP_1           0x51
#define OP_16          0x60
#define OP_DUP         0x76
#define OP_EQUAL       0x87
#define OP_EQUALVERIFY 0x88
#define OP_HASH160     0xa9
#define OP_CHECKSIG    0xac

		enum SignType {
			SignTypeInvalid    = 0,
			SignTypeStandard   = 0xAC,
			SignTypeDID        = 0xAD,
			SignTypeMultiSign  = 0xAE,
			SignTypeCrossChain = 0xAF,
			SignTypeDestroy    = 0xAA,
		};

		enum Prefix {
			PrefixStandard   = 0x21,
			PrefixMultiSign  = 0x12,
			PrefixCrossChain = 0x4B,
			PrefixCRExpenses = 0x1C,
			PrefixDeposit    = 0x1F,
			PrefixIDChain    = 0x67,
			PrefixDestroy    = 0,
		};

		class Address {
		public:
			Address();

			explicit Address(const std::string &address);

			explicit Address(const uint168 &programHash);

			Address(Prefix prefix, const bytes_t &pubkey, bool did = false);

			Address(Prefix prefix, const std::vector<bytes_t> &pubkey, uint8_t m, bool did = false);

			Address(const Address &address);

			~Address();

			bool Valid() const;

			bool IsIDAddress() const;

			std::string String() const;

			const uint168 &ProgramHash() const;

			void SetProgramHash(const uint168 &programHash);

			SignType PrefixToSignType(Prefix prefix) const;

			const bytes_t &RedeemScript() const;

			void SetRedeemScript(Prefix prefix, const bytes_t &code);

			bool ChangePrefix(Prefix prefix);

			void ConvertToDID();

			bool operator<(const Address &address) const;

			bool operator==(const Address &address) const;

			bool operator==(const std::string &address) const;

			bool operator!=(const Address &address) const;

			bool operator!=(const std::string &address) const;

			Address &operator=(const Address &address);

		private:

			bool Compare(const bytes_t &a, const bytes_t &b) const;

			void GenerateCode(Prefix prefix, const std::vector<bytes_t> &pubkeys, uint8_t m, bool did = false);

			void GenerateProgramHash(Prefix prefix);

			bool CheckValid();

		private:
			uint168 _programHash;
			bytes_t _code;
			std::string _str;
			bool _isValid;
		};

		typedef boost::shared_ptr<Address> AddressPtr;
		typedef struct {
			bool operator() (const AddressPtr &x, const AddressPtr &y) const {
				return *x < *y;
			}
		} AddressCompare;
		typedef std::set<AddressPtr, AddressCompare> AddressSet;
		typedef std::vector<AddressPtr> AddressArray;

	}
}


#endif //__ELASTOS_SDK_ADDRESS_H__
