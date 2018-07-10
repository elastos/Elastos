// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BTCKEY_H__
#define __ELASTOS_SDK_BTCKEY_H__

//Add * accorded with breadwallet by zhangcl 791398105@qq.com

#include <boost/filesystem.hpp>
#include <openssl/obj_mac.h>
#include <BRInt.h>
#include <string>

#include "BRBIP32Sequence.h"
#include "CMemBlock.h"

#define BIP32_SEED_KEY "ELA SEED"

namespace Elastos {
	namespace ElaWallet {
		class BTCKey {
		public:
			/** Generates pair of key for ECDSA.
 			 *  \param  privKey  CMemBlock for returned PublicKey.
 			 *  \param  pubKey CMemBlock for returned PrivateKey.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool
			generateKey(CMBlock &privKey, CMBlock &pubKey, int nid = NID_secp256k1);

			/** Signatures data for ECDSA.
 			 *  \param  privKey CMemBlock for Sign's PrivateKey.
 			 *  \param  data CMemBlock to be signed.
 			 * 	\param  signedData CMemBlock for returned SignedData.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */

			static bool ECDSASign(const CMBlock &privKey, const CMBlock &data, CMBlock &signedData,
								  int nid = NID_secp256k1);

			/** Verifys signature's for ECDSA.
 			 *  \param  pubKey CMemBlock for verify's PublicKey.
 			 *  \param  data CMemBlock to be signed.
 			 * 	\param  signedData CMemBlock for SignedData.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */

			static bool ECDSAVerify(const CMBlock &pubKey, const CMBlock &data, const CMBlock &signedData,
									int nid = NID_secp256k1);

			/** Signatures data for ECDSA.
 			 *  \param  privKey CMemBlock for Sign's PrivateKey.
 			 *  \param  data CMemBlock to be signed.
 			 * 	\param  signedData CMemBlock for returned SignedData.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool
			ECDSACompactSign(const CMBlock &privKey, const CMBlock &data, CMBlock &signedData,
							 int nid = NID_secp256k1);

			/** Verifys signature's for ECDSA.
 			 *  \param  pubKey CMemBlock for verify's PublicKey.
 			 *  \param  data CMemBlock for PlainText.
 			 * 	\param  signedData CMemBlock for SignedData.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool
			ECDSACompactVerify(const CMBlock &pubKey, const CMBlock &data, const CMBlock &signedData,
							   int nid = NID_secp256k1);

			/** Signatures data for ECDSA.
		  	 *  \param  privKey CMemBlock for Sign's PrivateKey.
		     *  \param  md UInt256 to be signed.
		     * 	\param  signedData CMemBlock for returned SignedData.
		     * 	\param  nid int for style of ECDSA.
		     *  \return true on success and false on failure.
		     */
			static bool
			ECDSACompactSign_sha256(const CMBlock &privKey, const UInt256 &md, CMBlock &signedData,
									int nid = NID_secp256k1);

			/** Verifys signature's for ECDSA.
 			 *  \param  pubKey CMemBlock for verify's PublicKey.
 			 *  \param  md UInt256 for PlainText.
 			 * 	\param  signedData CMemBlock for SignedData.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool
			ECDSACompactVerify_sha256(const CMBlock &pubKey, const UInt256 &md, const CMBlock &signedData,
									  int nid = NID_secp256k1);

			/** Signatures data for ECDSA.
		  	 *  \param  privKey CMemBlock for Sign's PrivateKey.
		     *  \param  md UInt256 to be signed.
		     * 	\param  signedData CMemBlock for returned SignedData.
		     * 	\param  nid int for style of ECDSA.
		     *  \return true on success and false on failure.
		     */
			static bool
			ECDSA65Sign_sha256(const CMBlock &privKey, const UInt256 &md, CMBlock &signedData,
							   int nid = NID_secp256k1);

			/** Verifys signature's for ECDSA.
 			 *  \param  pubKey CMemBlock for verify's PublicKey.
 			 *  \param  md UInt256 for PlainText.
 			 * 	\param  signedData CMemBlock for SignedData.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool
			ECDSA65Verify_sha256(const CMBlock &pubKey, const UInt256 &md, const CMBlock &signedData,
								 int nid = NID_secp256k1);

			/** Get MasterPrivateKey' seed for ECDSA.
 			 *  \param  phrase std::string for mnemonic.
 			 *  \param  phrasePassword CMemBlock for mnemonic's password.
 			 * 	\param  i18nPath boost::filesystem::path for WordsList table path.
 			 * 	\param  language std::string for Words's language.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return seed CMemBlock.
 			 */
			static CMBlock
			getPrivKeySeed(const std::string &phrase, const std::string &phrasePassword,
						   const boost::filesystem::path &i18nPath, const std::string &language = "english",
						   int nid = NID_secp256k1);

			/** Get MasterPrivateKey from seed for ECDSA.
 			 *  \param  seed CMemBlock varied from getPrivKeySeed for ECDSA.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return MasterPrivateKey CMemBlock.
 			 */
			static CMBlock
			getMasterPrivkey(const CMBlock &seed, int nid = NID_secp256k1);

			/** Get PublicKey from corresponding to PrivateKey for ECDSA.
 			 *  \param  privKey CMemBlock vary from getMasterPrivkey or generateKey for ECDSA.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return PublicKey CMemBlock.
 			 */
			static CMBlock getPubKeyFromPrivKey(const CMBlock &privKey, int nid = NID_secp256k1);

			/** Check PublicKey validation for ECDSA.
 			 *  \param  pubKey CMemBlock for PrivateKey.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool PublickeyIsValid(const CMBlock &pubKey, int nid = NID_secp256k1);

			/** Check PrivateKey/PublicKey validation for ECDSA.
 			 *  \param  privKey  CMemBlock for PrivateKey.
 			 *  \param  pubKey CMemBlock for PublicKey.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return true on success and false on failure.
 			 */
			static bool
			KeyIsValid(const CMBlock &privKey, const CMBlock &pubKey, int nid = NID_secp256k1);

			/** Get Derived PublicKey from MasterPublicKey for ECDSA.
 			 *  \param  pubKey CMemBlock for MasterPublicKey.
 			 *  \param  chain uint32_t choosed to SEQUENCE_EXTERNAL_CHAIN/SEQUENCE_INTERNAL_CHAIN.
 			 *  \param  index uint32_t indicated number of deriving PublicKey.
 			 *  \param  chainCode UInt256 recommended to use default when haves none chainCode.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return DerivedPublicKey CMemBlock.
 			 */
			static CMBlock
			getDerivePubKey(const CMBlock &pubKey, uint32_t chain, uint32_t index,
							UInt256 chainCode = UINT256_ZERO, int nid = NID_secp256k1);

			/** Get Derived PrivateKey from seed for ECDSA.
 			 *  \param  seed varied from getPrivKeySeed.
 			 *  \param  chain uint32_t choosed to SEQUENCE_EXTERNAL_CHAIN/SEQUENCE_INTERNAL_CHAIN
 			 *  \param  index uint32_t indicated number of deriving PublicKey
 			 *  \param  chainCode UInt256 recommended to use default when haves none chainCode
 			 *  \param  useChainCode bool recommended to use false.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return DerivedPrivateKey CMemBlock.
 			 */
			static CMBlock
			getDerivePrivKey(const CMBlock &seed, uint32_t chain, uint32_t index, UInt256 &chainCode,
							 bool useChainCode = false, int nid = NID_secp256k1);

			/** Get Derived PrivateKey from seed for ECDSA.
 			 *  \param  seed varied from getPrivKeySeed.
 			 *  \param  chainCode UInt256 recommended to use default when haves none chainCode
 			 *  \param  useChainCode bool recommended to use false.
 			 * 	\param  nid int for style of ECDSA.
 			 * 	\param  depth int for Derive deep.
 			 *  \return DerivedPrivateKey CMemBlock.
 			 */
			static CMBlock
			getDerivePrivKey_depth(const CMBlock &seed, UInt256 &chainCode, bool useChainCode, int nid,
								   int depth, ...);

			/** Get Derived PrivateKey from seed for ECDSA.
 			 *  \param  seed varied from getPrivKeySeed.
 			 *  \param  chainCode UInt256 recommended to use default when haves none chainCode
 			 *  \param  useChainCode bool recommended to use false.
 			 * 	\param  nid int for style of ECDSA.
 			 * 	\param  depth int for Derive deep.
 			 * 	\param  ap va_list for params
 			 *  \return DerivedPrivateKey CMemBlock.
 			 */
			static CMBlock
			getDerivePrivKey_depth(const CMBlock &seed, UInt256 &chainCode, bool useChainCode, int nid,
								   int depth, va_list ap);

			/** Get Derived PrivateKey list from seed for ECDSA.
			 *  \param  privKeys std::vector<CMBlock > initials to size equal to
			 *                   sizeof(indexes)/sizeof(uint32_t) for returned Childs' PrivateKey(Derived keys).
 			 *  \param  seed varied from getPrivKeySeed.
 			 *  \param  chain uint32_t choosed to SEQUENCE_EXTERNAL_CHAIN/SEQUENCE_INTERNAL_CHAIN.
 			 *  \param  indexes uint32_t[] containing indexes responded to index in getDerivePrivKey.
 			 *  \param  chainCode UInt256 recommended to use default when has none chainCode.
 			 *  \param  useChainCode bool recommended to use false.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return void.
 			 */
			static void
			getDerivePrivKey(std::vector<CMBlock> &privKeys, const CMBlock &seed, uint32_t chain,
							 const uint32_t indexes[], UInt256 &chainCode, bool useChainCode = false,
							 int nid = NID_secp256k1);

			/** Get Derived PrivateKey from MasterPrivateKey for ECDSA.
 			 *  \param  privKey CMemBlock on behalf of MasterPrivateKey varied from getMasterPrivkey/generateKey.
 			 *  \param  chain uint32_t choosed to SEQUENCE_EXTERNAL_CHAIN/SEQUENCE_INTERNAL_CHAIN.
 			 *  \param  index uint32_t indicated number of deriving PublicKey.
 			 *  \param  chainCode UInt256 recommended to use default when has none chainCode.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return DerivedPrivateKey CMemBlock.
 			 */
			static CMBlock
			getDerivePrivKey_Secret(const CMBlock &privKey, uint32_t chain, uint32_t index,
									UInt256 chainCode = UINT256_ZERO, int nid = NID_secp256k1);

			/** Get Derived PrivateKey from MasterPrivateKey for ECDSA.
 			 *  \param  privKey CMemBlock on behalf of MasterPrivateKey varied from getMasterPrivkey/generateKey.
 			 *  \param  chainCode UInt256 recommended to use default when has none chainCode.
 			 * 	\param  nid int for style of ECDSA.
 			 * 	\param  depth int for Derive deep.
 			 *  \return DerivedPrivateKey CMemBlock.
 			 */
			static CMBlock
			getDerivePrivKey_Secret_depth(const CMBlock &privKey, UInt256 chainCode, bool useChainCode,
										  int nid, int depth, ...);

			/** Get Derived PrivateKey from MasterPrivateKey for ECDSA.
 			 *  \param  privKey CMemBlock on behalf of MasterPrivateKey varied from getMasterPrivkey/generateKey.
 			 *  \param  chainCode UInt256 recommended to use default when has none chainCode.
 			 * 	\param  nid int for style of ECDSA.
 			 * 	\param  depth int for Derive deep.
 			 * 	\param  ap va_list for params
 			 *  \return DerivedPrivateKey CMemBlock.
 			 */
			static CMBlock
			getDerivePrivKey_Secret_depth(const CMBlock &privKey, UInt256 chainCode, bool useChainCode,
										  int nid, int depth, va_list ap);

			/** Get Derived PrivateKey list from MasterPrivateKey for ECDSA.
			 *  \param  privKeys std::vector<CMBlock > initials to size equal to
			 *                   sizeof(indexes)/sizeof(uint32_t) for returned Childs' PrivateKey(Derived keys).
 			 *  \param  privKey CMemBlock on behalf of MasterPrivateKey varied from getMasterPrivkey/generateKey.
 			 *  \param  chain uint32_t choosed to SEQUENCE_EXTERNAL_CHAIN/SEQUENCE_INTERNAL_CHAIN.
 			 *  \param  indexes uint32_t[] containing indexes responded to index in getDerivePrivKey.
 			 *  \param  chainCode UInt256 recommended to use default when has none chainCode.
 			 * 	\param  nid int for style of ECDSA.
 			 *  \return void.
 			 */
			static void
			getDerivePrivKey_Secret(std::vector<CMBlock> &privKeys, const CMBlock &privKey,
									uint32_t chain, const uint32_t indexes[], UInt256 chainCode = UINT256_ZERO,
									int nid = NID_secp256k1);
		};
	}
}


#endif //__ELASTOS_SDK_BTCKEY_H__
