package version

import (
	"errors"
	"math"

	"fmt"
	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
)

type TxVersion interface {
	GetVersion() byte

	CheckOutputPayload(txType types.TransactionType, output *types.Output) error
	CheckOutputProgramHash(programHash common.Uint168) error
	CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error
	CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error
	CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error
	CheckTxHasNoPrograms(tx *types.Transaction) error
}

type TxVersionMain struct {
}

func (v *TxVersionMain) GetVersion() byte {
	return 9
}

func (v *TxVersionMain) CheckOutputPayload(txType types.TransactionType, output *types.Output) error {
	// Vote information can only be placed in TransferAsset transaction.
	if txType == types.TransferAsset {
		switch output.OutputType {
		case types.DefaultOutput:
		case types.VoteOutput:
		default:
			goto ERRMATCH
		}
	} else {
		switch output.OutputType {
		case types.DefaultOutput:
		default:
			goto ERRMATCH
		}
	}

	if err := output.OutputPayload.Validate(); err != nil {
		return err
	} else {
		return nil
	}

ERRMATCH:
	return errors.New("transaction type dose not match the output payload type")
}

func (v *TxVersionMain) CheckOutputProgramHash(programHash common.Uint168) error {
	var empty = common.Uint168{}
	if programHash.IsEqual(empty) {
		return nil
	}

	prefix := contract.PrefixType(programHash[0])
	switch prefix {
	case contract.PrefixStandard:
	case contract.PrefixMultiSig:
	case contract.PrefixCrossChain:
	case contract.PrefixDeposit:
	default:
		return errors.New("invalid program hash prefix")
	}

	addr, err := programHash.ToAddress()
	if err != nil {
		return errors.New("invalid program hash")
	}
	_, err = common.Uint168FromAddress(addr)
	if err != nil {
		return errors.New("invalid program hash")
	}

	return nil
}

func (v *TxVersionMain) CheckCoinbaseMinerReward(tx *types.Transaction, totalReward common.Fixed64) error {
	minerReward := tx.Outputs[1].Value
	if common.Fixed64(minerReward) < common.Fixed64(float64(totalReward)*0.35) {
		return errors.New("Reward to miner in coinbase < 35%")
	}

	return nil
}

func (v *TxVersionMain) CheckCoinbaseArbitratorsReward(coinbase *types.Transaction, rewardInCoinbase common.Fixed64) error {
	outputAddressMap := make(map[common.Uint168]common.Fixed64)

	for i := 2; i < len(coinbase.Outputs); i++ {
		outputAddressMap[coinbase.Outputs[i].ProgramHash] = coinbase.Outputs[i].Value
	}

	arbitratorsHashes := blockchain.DefaultLedger.Arbitrators.GetArbitratorsProgramHashes()
	candidatesHashes := blockchain.DefaultLedger.Arbitrators.GetCandidatesProgramHashes()
	if len(arbitratorsHashes)+len(candidatesHashes) != len(coinbase.Outputs)-2 {
		return errors.New("Coinbase output count not match.")
	}

	dposTotalReward := common.Fixed64(float64(rewardInCoinbase) * 0.35)
	totalBlockConfirmReward := float64(dposTotalReward) * 0.25
	totalTopProducersReward := float64(dposTotalReward) * 0.75
	individualBlockConfirmReward := common.Fixed64(math.Floor(totalBlockConfirmReward / float64(len(arbitratorsHashes))))
	individualProducerReward := common.Fixed64(math.Floor(totalTopProducersReward / float64(len(arbitratorsHashes)+len(candidatesHashes))))

	for _, v := range arbitratorsHashes {

		amount, ok := outputAddressMap[*v]
		if !ok {
			return errors.New("Unknown dpos reward address.")
		}

		if amount != individualProducerReward+individualBlockConfirmReward {
			return errors.New("Incorrect dpos reward amount.")
		}
	}

	for _, v := range candidatesHashes {

		amount, ok := outputAddressMap[*v]
		if !ok {
			return errors.New("Unknown dpos reward address.")
		}

		if amount != individualProducerReward {
			return errors.New("Incorrect dpos reward amount.")
		}
	}

	return nil
}

func (v *TxVersionMain) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error {
	programHashes := make(map[common.Uint168]struct{})
	for _, v := range references {
		programHashes[v.ProgramHash] = struct{}{}
	}

	pds := make(map[string]struct{})
	for _, p := range producers {
		pds[common.BytesToHexString(p)] = struct{}{}
	}

	for _, o := range outputs {
		if o.OutputType == types.VoteOutput {
			if _, ok := programHashes[o.ProgramHash]; !ok {
				return errors.New("Invalid vote output")
			}
			payload, ok := o.OutputPayload.(*outputpayload.VoteOutput)
			if !ok {
				return errors.New("Invalid vote output payload")
			}
			for _, content := range payload.Contents {
				if content.VoteType == outputpayload.Delegate {
					for _, candidate := range content.Candidates {
						if _, ok := pds[common.BytesToHexString(candidate)]; !ok {
							return fmt.Errorf("Invalid vote output payload candidate: %s", common.BytesToHexString(candidate))
						}
					}
				}
			}
		}
	}

	return nil
}

func (v *TxVersionMain) CheckTxHasNoPrograms(tx *types.Transaction) error {
	if len(tx.Programs) != 0 {
		return errors.New("Transaction should have no programs.")
	}

	return nil
}
