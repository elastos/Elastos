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
#include "SidechainSubWallet.h"
#include "MasterWallet.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <SpvService/Config.h>
#include <WalletCore/CoinInfo.h>
#include <Plugin/Transaction/TransactionOutput.h>

#include <vector>
#include <map>

namespace Elastos {
	namespace ElaWallet {

		SidechainSubWallet::SidechainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent,
											   const std::string &netType) :
				ElastosBaseSubWallet(info, config, parent, netType) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(
			const nlohmann::json &inputsJson,
			const std::string &amount,
			const std::string &mainChainAddress,
			const std::string &fee,
			const std::string &memo) {

			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("amount: {}", amount);
			ArgInfo("mainChainAddr: {}", mainChainAddress);
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			ErrorChecker::CheckBigIntAmount(amount);

			BigInt bgAmount, feeAmount;
			bgAmount.setDec(amount);
			feeAmount.setDec(fee);

			PayloadPtr payload = nullptr;
			try {
				TransferInfo info(mainChainAddress, 0, bgAmount);
				payload = PayloadPtr(new TransferCrossChainAsset({info}));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "main chain message error: " + std::string(e.what()));
			}

			OutputArray outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount + DEPOSIT_OR_WITHDRAW_FEE, Address(ELA_SIDECHAIN_DESTROY_ADDR))));

			TransactionPtr tx = wallet->CreateTransaction(Transaction::transferCrossChainAsset, payload, utxo, outputs, memo, feeAmount);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

	}
}