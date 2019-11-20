// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainSubWallet.h"
#include "MasterWallet.h"

#include <Common/ErrorChecker.h>
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <SpvService/Config.h>
#include <WalletCore/CoinInfo.h>
#include <Plugin/Transaction/TransactionOutput.h>

#include <BRAddress.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		SidechainSubWallet::SidechainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent,
											   const std::string &netType) :
				SubWallet(info, config, parent, netType) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(
			const std::string &fromAddress,
			const std::string &amount,
			const std::string &mainChainAddress,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("mainChainAddr: {}", mainChainAddress);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount;
			bgAmount.setDec(amount);

			PayloadPtr payload = nullptr;
			try {
				TransferInfo info(mainChainAddress, 0, bgAmount);
				payload = PayloadPtr(new TransferCrossChainAsset({info}));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "main chain message error: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount + _config->MinFee(), Address(ELA_SIDECHAIN_DESTROY_ADDR))));

			TransactionPtr tx = CreateTx(Transaction::transferCrossChainAsset, payload, fromAddress, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string SidechainSubWallet::GetGenesisAddress() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::string address = _config->GenesisAddress();

			ArgInfo("r => {}", address);
			return address;
		}

	}
}