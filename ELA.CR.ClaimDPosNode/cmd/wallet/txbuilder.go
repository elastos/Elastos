// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package wallet

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"math"
	"math/rand"
	"os"
	"strconv"

	"github.com/elastos/Elastos.ELA/account"
	cmdcom "github.com/elastos/Elastos.ELA/cmd/common"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	pg "github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/urfave/cli"
)

type OutputInfo struct {
	Recipient string
	Amount    *common.Fixed64
}

type CrossChainOutput struct {
	Recipient         string
	Amount            *common.Fixed64
	CrossChainAddress string
}

func CreateTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")

	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}

	fee, err := common.StringToFixed64(feeStr)
	if err != nil {
		return errors.New("invalid transaction fee")
	}

	from := c.String("from")

	outputs := make([]*OutputInfo, 0)
	to := c.String("to")
	amountStr := c.String("amount")
	toMany := c.String("tomany")
	if toMany != "" {
		if to != "" {
			return errors.New("'--to' cannot be specified when specify '--tomany' option")
		}
		if amountStr != "" {
			return errors.New("'--amount' cannot be specified when specify '--tomany' option")
		}
		outputs, err = parseMultiOutput(toMany)
		if err != nil {
			return err
		}
	} else {
		if amountStr == "" {
			return errors.New("use --amount to specify transfer amount")
		}
		amount, err := common.StringToFixed64(amountStr)
		if err != nil {
			return errors.New("invalid transaction amount")
		}
		if to == "" {
			return errors.New("use --to to specify recipient")
		}
		outputs = []*OutputInfo{{to, amount}}
	}

	outputLockStr := c.String("outputlock")
	outputLock := uint64(0)
	if outputLockStr != "" {
		outputLock, err = strconv.ParseUint(outputLockStr, 10, 32)
		if err != nil {
			return errors.New("invalid output lock height")
		}
	}

	txLockStr := c.String("txlock")
	txLock := uint64(0)
	if txLockStr != "" {
		txLock, err = strconv.ParseUint(txLockStr, 10, 32)
		if err != nil {
			return errors.New("invalid transaction lock height")
		}
	}

	var txn *types.Transaction
	txn, err = createTransaction(walletPath, from, *fee, uint32(outputLock),
		uint32(txLock), outputs...)
	if err != nil {
		return errors.New("create transaction failed: " + err.Error())
	}

	OutputTx(0, 1, txn)

	return nil
}

func getSender(walletPath string, from string) (*account.AccountData, error) {
	var sender *account.AccountData
	mainAccount, err := account.GetWalletMainAccountData(walletPath)
	if err != nil {
		return nil, err
	}

	if from == "" {
		from = mainAccount.Address
		sender = mainAccount
	} else {
		storeAccounts, err := account.GetWalletAccountData(walletPath)
		if err != nil {
			return nil, err
		}
		for _, acc := range storeAccounts {
			if from == acc.Address {
				sender = &acc
				break
			}
		}
		if sender == nil {
			return nil, errors.New(from + " is not local account")
		}
	}

	return sender, nil
}

func createInputs(fromAddr string, totalAmount common.Fixed64) ([]*types.Input,
	[]*types.Output, error) {
	UTXOs, err := getUTXOsByAmount(fromAddr, totalAmount)
	if err != nil {
		return nil, nil, err
	}

	var txInputs []*types.Input
	var changeOutputs []*types.Output
	for _, utxo := range UTXOs {
		txIDReverse, _ := hex.DecodeString(utxo.TxID)
		txID, _ := common.Uint256FromBytes(common.BytesReverse(txIDReverse))
		sequence := math.MaxUint32
		if utxo.OutputLock > 0 {
			sequence = math.MaxUint32 - 1
		}
		input := &types.Input{
			Previous: types.OutPoint{
				TxID:  *txID,
				Index: uint16(utxo.VOut),
			},
			Sequence: uint32(sequence),
		}
		txInputs = append(txInputs, input)
		amount, err := common.StringToFixed64(utxo.Amount)
		if err != nil {
			return nil, nil, err
		}
		programHash, err := common.Uint168FromAddress(fromAddr)
		if err != nil {
			return nil, nil, err
		}
		if *amount < totalAmount {
			totalAmount -= *amount
		} else if *amount == totalAmount {
			totalAmount = 0
			break
		} else if *amount > totalAmount {
			change := &types.Output{
				AssetID:     *account.SystemAssetID,
				Value:       *amount - totalAmount,
				OutputLock:  uint32(0),
				ProgramHash: *programHash,
				Type:        types.OTNone,
				Payload:     &outputpayload.DefaultOutput{},
			}
			changeOutputs = append(changeOutputs, change)
			totalAmount = 0
			break
		}
	}
	if totalAmount > 0 {
		return nil, nil, errors.New("[Wallet], Available token is not enough")
	}

	return txInputs, changeOutputs, nil
}

func createNormalOutputs(outputs []*OutputInfo, fee common.Fixed64, lockedUntil uint32) ([]*types.Output, common.Fixed64, error) {
	var totalAmount = common.Fixed64(0) // The total amount will be spend
	var txOutputs []*types.Output       // The outputs in transaction
	totalAmount += fee                  // Add transaction fee

	for _, output := range outputs {
		recipient, err := common.Uint168FromAddress(output.Recipient)
		if err != nil {
			return nil, 0, errors.New(fmt.Sprint("invalid receiver address: ", output.Recipient, ", error: ", err))
		}

		txOutput := &types.Output{
			AssetID:     *account.SystemAssetID,
			ProgramHash: *recipient,
			Value:       *output.Amount,
			OutputLock:  lockedUntil,
			Type:        types.OTNone,
			Payload:     &outputpayload.DefaultOutput{},
		}
		totalAmount += *output.Amount
		txOutputs = append(txOutputs, txOutput)
	}

	return txOutputs, totalAmount, nil
}

func createVoteOutputs(output *OutputInfo, candidateList []string) ([]*types.Output, error) {
	var txOutputs []*types.Output
	recipient, err := common.Uint168FromAddress(output.Recipient)
	if err != nil {
		return nil, errors.New(fmt.Sprint("invalid receiver address: ", output.Recipient, ", error: ", err))
	}

	// create vote output payload
	var cv []outputpayload.CandidateVotes
	for _, candidateHex := range candidateList {
		candidateBytes, err := common.HexStringToBytes(candidateHex)
		if err != nil {
			return nil, err
		}
		cv = append(cv, outputpayload.CandidateVotes{
			Candidate: candidateBytes,
		})
	}
	voteContent := outputpayload.VoteContent{
		VoteType:       outputpayload.Delegate,
		CandidateVotes: cv,
	}
	voteOutput := outputpayload.VoteOutput{
		Version: 0,
		Contents: []outputpayload.VoteContent{
			voteContent,
		},
	}

	txOutput := &types.Output{
		AssetID:     *account.SystemAssetID,
		ProgramHash: *recipient,
		Value:       *output.Amount,
		OutputLock:  0,
		Type:        types.OTVote,
		Payload:     &voteOutput,
	}
	txOutputs = append(txOutputs, txOutput)

	return txOutputs, nil
}

func createTransaction(walletPath string, from string, fee common.Fixed64, outputLock uint32,
	txLock uint32, outputs ...*OutputInfo) (*types.Transaction, error) {
	// check output
	if len(outputs) == 0 {
		return nil, errors.New("invalid transaction target")
	}

	// get sender in wallet by from address
	sender, err := getSender(walletPath, from)
	if err != nil {
		return nil, err
	}

	// create outputs
	txOutputs, totalAmount, err := createNormalOutputs(outputs, fee, outputLock)
	if err != nil {
		return nil, err
	}

	// create inputs
	txInputs, changeOutputs, err := createInputs(sender.Address, totalAmount)
	if err != nil {
		return nil, err
	}
	txOutputs = append(txOutputs, changeOutputs...)

	redeemScript, err := common.HexStringToBytes(sender.RedeemScript)
	if err != nil {
		return nil, err
	}
	// create attributes
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	txAttributes := make([]*types.Attribute, 0)
	txAttributes = append(txAttributes, &txAttr)

	// create program
	var txProgram = &pg.Program{
		Code:      redeemScript,
		Parameter: nil,
	}
	return &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.TransferAsset,
		Payload:    &payload.TransferAsset{},
		Attributes: txAttributes,
		Inputs:     txInputs,
		Outputs:    txOutputs,
		Programs:   []*pg.Program{txProgram},
		LockTime:   txLock,
	}, nil
}

func CreateActivateProducerTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}

	client, err := account.Open(walletPath, password)
	if err != nil {
		return err
	}

	var acc *account.Account
	var nodePublicKey []byte

	nodePublicKeyStr := c.String("nodepublickey")
	if nodePublicKeyStr != "" {
		nodePublicKey, err = common.HexStringToBytes(nodePublicKeyStr)
		if err != nil {
			return err
		}
		codeHash, err := contract.PublicKeyToStandardCodeHash(nodePublicKey)
		if err != nil {
			return err
		}
		acc = client.GetAccountByCodeHash(*codeHash)
		if acc == nil {
			return errors.New("no available account in wallet")
		}
	} else {
		acc = client.GetMainAccount()
		if contract.GetPrefixType(acc.ProgramHash) != contract.PrefixStandard {
			return errors.New("main account is not a standard account")
		}
		nodePublicKey, err = acc.PublicKey.EncodePoint(true)
		if err != nil {
			return err
		}
	}

	buf := new(bytes.Buffer)
	apPayload := &payload.ActivateProducer{
		NodePublicKey: nodePublicKey,
	}
	if err = apPayload.SerializeUnsigned(buf, payload.ActivateProducerVersion); err != nil {
		return err
	}
	signature, err := acc.Sign(buf.Bytes())
	if err != nil {
		return err
	}
	apPayload.Signature = signature

	txn := &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.ActivateProducer,
		Payload:    apPayload,
		Attributes: nil,
		Inputs:     nil,
		Outputs:    nil,
		Programs:   []*pg.Program{},
		LockTime:   0,
	}

	OutputTx(0, 0, txn)

	return nil
}

func CreateCRCProposalWithdrawTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")
	if walletPath == "" {
		return errors.New("use --wallet to specify wallet path")
	}
	password, err := cmdcom.GetFlagPassword(c)
	if err != nil {
		return err
	}
	fmt.Printf("walletPath %s, password %s \n", walletPath, password)

	proposalHashStr := c.String("proposalhash")
	if proposalHashStr == "" {
		return errors.New("use --proposalhash to specify transfer proposalhash")
	}
	CRCCommiteeAddr := c.String("crccommiteeaddr")
	if CRCCommiteeAddr == "" {
		return errors.New("use --crccommiteeaddr to specify from address")
	}
	amountStr := c.String("amount")
	if amountStr == "" {
		return errors.New("use --amount to specify transfer amount")
	}
	amount, err := common.StringToFixed64(amountStr)
	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}
	fee, err := common.StringToFixed64(feeStr)

	fmt.Printf("proposalhash:%s, fee:%s amout:%s CRCCommiteeAddr:%s\n",
		proposalHashStr, feeStr, amountStr, CRCCommiteeAddr)
	*amount -= *fee

	client, err := account.Open(walletPath, password)
	if err != nil {
		return err
	}
	var acc *account.Account
	var OwnerPublicKey []byte

	acc = client.GetMainAccount()
	if contract.GetPrefixType(acc.ProgramHash) != contract.PrefixStandard {
		return errors.New("main account is not a standard account")
	}
	OwnerPublicKey, err = acc.PublicKey.EncodePoint(true)
	if err != nil {
		return err
	}
	proposalHash, err2 := common.Uint256FromHexString(proposalHashStr)
	if err2 != nil {
		return err2
	}
	crcProposalWithdraw := &payload.CRCProposalWithdraw{
		ProposalHash:   *proposalHash,
		OwnerPublicKey: OwnerPublicKey,
	}

	signBuf := new(bytes.Buffer)
	crcProposalWithdraw.SerializeUnsigned(signBuf, payload.CRCProposalWithdrawDefault)
	signature, err := acc.Sign(signBuf.Bytes())
	if err != nil {
		return err
	}
	crcProposalWithdraw.Signature = signature

	recipient := c.String("to")
	outputs := make([]*OutputInfo, 0)
	outpusInfo := &OutputInfo{
		Recipient: recipient,
		Amount:    amount,
	}
	outputs = append(outputs, outpusInfo)
	// create outputs
	txOutputs, totalAmount, err := createNormalOutputs(outputs, *fee, uint32(0))
	if err != nil {
		return err
	}
	// create inputs from CRCCommiteeAddr
	txInputs, changeOutputs, err := createInputs(CRCCommiteeAddr, totalAmount)
	if err != nil {
		return err
	}
	txOutputs = append(txOutputs, changeOutputs...)

	txn := &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.CRCProposalWithdraw,
		Payload:    crcProposalWithdraw,
		Attributes: []*types.Attribute{},
		Inputs:     txInputs,
		Outputs:    txOutputs,
		Programs:   []*pg.Program{},
		LockTime:   0,
	}

	OutputTx(0, 0, txn)

	return nil
}

func CreateVoteTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")

	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}
	fee, err := common.StringToFixed64(feeStr)
	if err != nil {
		return errors.New("invalid transaction fee")
	}

	// calculate total amount
	amountStr := c.String("amount")
	if amountStr == "" {
		return errors.New("use --amount to specify transfer amount")
	}
	amount, err := common.StringToFixed64(amountStr)
	if err != nil {
		return errors.New("invalid transaction amount")
	}
	totalAmount := *fee + *amount

	// get sender from wallet by from address
	from := c.String("from")
	sender, err := getSender(walletPath, from)
	if err != nil {
		return err
	}

	// get candidate list from file
	candidatePath := c.String("for")
	candidateList, err := parseCandidates(candidatePath)
	if err != nil {
		return err
	}

	// create outputs
	txOutputs, err := createVoteOutputs(&OutputInfo{
		Recipient: sender.Address,
		Amount:    amount,
	}, candidateList)
	if err != nil {
		return err
	}

	// create inputs
	txInputs, changeOutputs, err := createInputs(sender.Address, totalAmount)
	if err != nil {
		return err
	}
	txOutputs = append(txOutputs, changeOutputs...)

	redeemScript, err := common.HexStringToBytes(sender.RedeemScript)
	if err != nil {
		return err
	}

	// create attributes
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	txAttributes := make([]*types.Attribute, 0)
	txAttributes = append(txAttributes, &txAttr)

	// create program
	var txProgram = &pg.Program{
		Code:      redeemScript,
		Parameter: nil,
	}

	txn := &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.TransferAsset,
		Payload:    &payload.TransferAsset{},
		Attributes: txAttributes,
		Inputs:     txInputs,
		Outputs:    txOutputs,
		Programs:   []*pg.Program{txProgram},
		LockTime:   0,
	}

	OutputTx(0, 1, txn)

	return nil
}

func CreateCrossChainTransaction(c *cli.Context) error {
	walletPath := c.String("wallet")

	from := c.String("from")
	to := c.String("to")
	if to == "" {
		return errors.New("use --to to specify a side chain address which want to recharge")
	}
	sAddress := c.String("saddress")
	if sAddress == "" {
		return errors.New("use --saddress to specify a locked address of side chain")
	}

	feeStr := c.String("fee")
	if feeStr == "" {
		return errors.New("use --fee to specify transfer fee")
	}
	fee, err := common.StringToFixed64(feeStr)
	if err != nil {
		return errors.New("invalid transaction fee")
	}

	amountStr := c.String("amount")
	if amountStr == "" {
		return errors.New("use --amount to specify transfer amount")
	}
	amount, err := common.StringToFixed64(amountStr)
	if err != nil {
		return errors.New("invalid transaction amount")
	}

	txn, err := createCrossChainTransaction(walletPath, from, *fee, 0, &CrossChainOutput{
		Recipient:         sAddress,
		Amount:            amount,
		CrossChainAddress: to,
	})
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	OutputTx(0, 1, txn)

	return nil
}

func createCrossChainTransaction(walletPath string, from string, fee common.Fixed64, lockedUntil uint32,
	crossChainOutputs ...*CrossChainOutput) (*types.Transaction, error) {
	// check output
	if len(crossChainOutputs) == 0 {
		return nil, errors.New("invalid transaction target")
	}

	outputs := make([]*OutputInfo, 0)
	perAccountFee := fee / common.Fixed64(len(crossChainOutputs))

	// create payload
	payload := &payload.TransferCrossChainAsset{}
	for index, output := range crossChainOutputs {
		payload.CrossChainAddresses = append(payload.CrossChainAddresses, output.CrossChainAddress)
		payload.OutputIndexes = append(payload.OutputIndexes, uint64(index))
		payload.CrossChainAmounts = append(payload.CrossChainAmounts, *output.Amount-perAccountFee)
		outputs = append(outputs, &OutputInfo{
			Recipient: output.Recipient,
			Amount:    output.Amount,
		})
	}

	// create outputs
	txOutputs, totalAmount, err := createNormalOutputs(outputs, fee, lockedUntil)
	if err != nil {
		return nil, err
	}

	// get sender in wallet by from address
	sender, err := getSender(walletPath, from)
	if err != nil {
		return nil, err
	}

	// create inputs
	txInputs, changeOutputs, err := createInputs(sender.Address, totalAmount)
	if err != nil {
		return nil, err
	}
	txOutputs = append(txOutputs, changeOutputs...)

	redeemScript, err := common.HexStringToBytes(sender.RedeemScript)
	if err != nil {
		return nil, err
	}
	// create attributes
	txAttr := types.NewAttribute(types.Nonce, []byte(strconv.FormatInt(rand.Int63(), 10)))
	txAttributes := make([]*types.Attribute, 0)
	txAttributes = append(txAttributes, &txAttr)

	// create program
	var txProgram = &pg.Program{
		Code:      redeemScript,
		Parameter: nil,
	}

	return &types.Transaction{
		Version:    types.TxVersion09,
		TxType:     types.TransferCrossChainAsset,
		Payload:    payload,
		Attributes: txAttributes,
		Inputs:     txInputs,
		Outputs:    txOutputs,
		Programs:   []*pg.Program{txProgram},
		LockTime:   0,
	}, nil
}
