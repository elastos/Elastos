package blocks

import (
	"bytes"
	"sort"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/version/verconf"
)

// Ensure blockV1 implement the BlockVersion interface.
var _ BlockVersion = (*blockV1)(nil)

// blockV1 represent the current block version.
type blockV1 struct {
	*blockV0
}

func (b *blockV1) GetVersion() uint32 {
	return 1
}

func (b *blockV1) GetNextOnDutyArbitrator(dutyChangedCount, offset uint32) []byte {
	arbitrators := b.cfg.Arbitrators.GetArbitrators()
	if len(arbitrators) == 0 {
		return nil
	}
	index := (dutyChangedCount + offset) % uint32(len(arbitrators))
	arbitrator := arbitrators[index]

	return arbitrator
}

func (b *blockV1) CheckConfirmedBlockOnFork(block *types.Block) error {
	if !b.cfg.Server.IsCurrent() {
		return nil
	}

	hash, err := b.cfg.ChainStore.GetBlockHash(block.Height)
	if err != nil {
		return err
	}

	anotherBlock, err := b.cfg.ChainStore.GetBlock(hash)
	if err != nil {
		return err
	}

	if block.Hash().IsEqual(anotherBlock.Hash()) {
		return nil
	}

	evidence, offset, err := b.generateBlockEvidence(block)
	if err != nil {
		return err
	}

	compareEvidence, compareOffset, err := b.generateBlockEvidence(anotherBlock)
	if err != nil {
		return err
	}

	// IllegalBlockEvidence tx shall not be created if view offset of two block
	// confirms is not equal.
	if offset > compareOffset {
		// do nothing if view offset of block on chain is less than current
		// block
		return nil
	} else if offset < compareOffset &&
		block.Hash().IsEqual(anotherBlock.Hash()) {
		// reorganize chain if view offset of block on chain is more than
		// current block, and these two blocks should be different
		return b.cfg.Chain.ReorganizeChain(block)
	}

	illegalBlocks := &payload.DPOSIllegalBlocks{
		CoinType:    payload.ELACoin,
		BlockHeight: block.Height,
	}

	asc := true
	if common.BytesToHexString(evidence.Header) >
		common.BytesToHexString(compareEvidence.Header) {
		asc = false
	}

	if asc {
		illegalBlocks.Evidence = *evidence
		illegalBlocks.CompareEvidence = *compareEvidence
	} else {
		illegalBlocks.Evidence = *compareEvidence
		illegalBlocks.CompareEvidence = *evidence
	}

	if err := blockchain.CheckDPOSIllegalBlocks(illegalBlocks); err != nil {
		return err
	}

	tx := &types.Transaction{
		Version:        types.TxVersion09,
		TxType:         types.IllegalBlockEvidence,
		PayloadVersion: payload.IllegalBlockVersion,
		Payload:        illegalBlocks,
		Attributes:     []*types.Attribute{},
		LockTime:       0,
		Programs:       []*program.Program{},
		Outputs:        []*types.Output{},
		Inputs:         []*types.Input{},
		Fee:            0,
	}
	if err := b.cfg.TxMemPool.AppendToTxPool(tx); err != nil {
		return err
	}

	return nil
}

func (b *blockV1) AssignCoinbaseTxRewards(block *types.Block, totalReward common.Fixed64) error {
	// PoW miners and DPoS are each equally allocated 35%. The remaining 30% goes to the Cyber Republic fund
	rewardCyberRepublic := common.Fixed64(float64(totalReward) * 0.3)
	rewardMergeMiner := common.Fixed64(float64(totalReward) * 0.35)
	rewardDposArbiter := common.Fixed64(totalReward) - rewardCyberRepublic - rewardMergeMiner
	block.Transactions[0].Outputs[0].Value = rewardCyberRepublic
	block.Transactions[0].Outputs[0].Type = types.OTNone
	block.Transactions[0].Outputs[0].Payload = &outputpayload.DefaultOutput{}

	block.Transactions[0].Outputs[1].Value = rewardMergeMiner
	block.Transactions[0].Outputs[1].Type = types.OTNone
	block.Transactions[0].Outputs[1].Payload = &outputpayload.DefaultOutput{}

	block.Transactions[0].Outputs = append(block.Transactions[0].Outputs, &types.Output{
		AssetID:     config.ELAAssetID,
		Value:       rewardDposArbiter,
		ProgramHash: blockchain.FoundationAddress,
		Type:        types.OTNone,
		Payload:     &outputpayload.DefaultOutput{},
	})

	return nil
}

func (b *blockV1) generateBlockEvidence(block *types.Block) (
	*payload.BlockEvidence, uint32, error) {
	headerBuf := new(bytes.Buffer)
	if err := block.Header.Serialize(headerBuf); err != nil {
		return nil, 0, err
	}

	confirm, err := b.cfg.ChainStore.GetConfirm(block.Hash())
	if err != nil {
		return nil, 0, err
	}
	confirmBuf := new(bytes.Buffer)
	if err = confirm.Serialize(confirmBuf); err != nil {
		return nil, 0, err
	}
	confirmSigners, err := b.getConfirmSigners(confirm)
	if err != nil {
		return nil, 0, err
	}

	return &payload.BlockEvidence{
		Header:       headerBuf.Bytes(),
		BlockConfirm: confirmBuf.Bytes(),
		Signers:      confirmSigners,
	}, confirm.Proposal.ViewOffset, nil
}

func (b *blockV1) getConfirmSigners(confirm *payload.Confirm) (
	[][]byte, error) {

	var signers []string
	for _, v := range confirm.Votes {
		signers = append(signers, common.BytesToHexString(v.Signer))
	}
	sort.Strings(signers)

	result := make([][]byte, 0)
	for _, v := range signers {
		signer, err := common.HexStringToBytes(v)
		if err != nil {
			return nil, err
		}

		result = append(result, signer)
	}

	return result, nil
}

func NewBlockV1(cfg *verconf.Config) *blockV1 {
	return &blockV1{NewBlockV0(cfg)}
}
