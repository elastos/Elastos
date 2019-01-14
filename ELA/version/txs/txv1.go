package txs

import (
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure txV1 implement the TxVersion interface.
var _ TxVersion = (*txV1)(nil)

type txV1 struct {
	*txV0
}

func (v *txV1) GetVersion() byte {
	return 1
}

func (v *txV1) CheckOutputProgramHash(programHash common.Uint168) error {
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

func (v *txV1) CheckVoteProducerOutputs(outputs []*types.Output, references map[*types.Input]*types.Output, producers [][]byte) error {
	programHashes := make(map[common.Uint168]struct{})
	for _, v := range references {
		programHashes[v.ProgramHash] = struct{}{}
	}

	pds := make(map[string]struct{})
	for _, p := range producers {
		pds[common.BytesToHexString(p)] = struct{}{}
	}

	for _, o := range outputs {
		if o.Type == types.OTVote {
			if _, ok := programHashes[o.ProgramHash]; !ok {
				return errors.New("Invalid vote output")
			}
			payload, ok := o.Payload.(*outputpayload.VoteOutput)
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

func NewTxV1(cfg *verconf.Config) *txV1 {
	return &txV1{NewTxV0(cfg)}
}
