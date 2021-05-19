package auxpow

import (
	"io"

	"github.com/elastos/Elastos.ELA/auxpow"
	"github.com/elastos/Elastos.ELA/common"
	ela "github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type SideAuxPow struct {
	SideAuxMerkleBranch []common.Uint256
	SideAuxMerkleIndex  int
	SideAuxBlockTx      ela.Transaction
	MainBlockHeader     ela.Header
}

func NewSideAuxPow(sideAuxMerkleBranch []common.Uint256,
	sideAuxMerkleIndex int,
	sideAuxBlockTx ela.Transaction,
	mainBlockHeader ela.Header) *SideAuxPow {

	return &SideAuxPow{
		SideAuxMerkleBranch: sideAuxMerkleBranch,
		SideAuxMerkleIndex:  sideAuxMerkleIndex,
		SideAuxBlockTx:      sideAuxBlockTx,
		MainBlockHeader:     mainBlockHeader,
	}
}

func (sap *SideAuxPow) Serialize(w io.Writer) error {
	err := sap.SideAuxBlockTx.Serialize(w)
	if err != nil {
		return err
	}

	err = common.WriteUint32(w, uint32(len(sap.SideAuxMerkleBranch)))
	if err != nil {
		return err
	}

	for _, branch := range sap.SideAuxMerkleBranch {
		err = branch.Serialize(w)
		if err != nil {
			return err
		}
	}

	err = common.WriteUint32(w, uint32(sap.SideAuxMerkleIndex))
	if err != nil {
		return err
	}

	return sap.MainBlockHeader.Serialize(w)
}

func (sap *SideAuxPow) Deserialize(r io.Reader) error {
	err := sap.SideAuxBlockTx.Deserialize(r)
	if err != nil {
		return err
	}

	count, err := common.ReadUint32(r)
	if err != nil {
		return err
	}

	sap.SideAuxMerkleBranch = make([]common.Uint256, 0, count)
	for i := uint32(0); i < count; i++ {
		var branch common.Uint256
		err = branch.Deserialize(r)
		if err != nil {
			return err
		}
		sap.SideAuxMerkleBranch = append(sap.SideAuxMerkleBranch, branch)
	}

	index, err := common.ReadUint32(r)
	if err != nil {
		return err
	}
	sap.SideAuxMerkleIndex = int(index)

	return sap.MainBlockHeader.Deserialize(r)
}

func (sap *SideAuxPow) SideAuxPowCheck(hashAuxBlock common.Uint256) bool {
	mainBlockHeader := sap.MainBlockHeader
	mainBlockHeaderHash := mainBlockHeader.Hash()
	if !mainBlockHeader.AuxPow.Check(&mainBlockHeaderHash, auxpow.AuxPowChainID) {
		return false
	}

	sideAuxPowMerkleRoot := auxpow.GetMerkleRoot(sap.SideAuxBlockTx.Hash(), sap.SideAuxMerkleBranch, sap.SideAuxMerkleIndex)
	if sideAuxPowMerkleRoot != sap.MainBlockHeader.MerkleRoot {
		return false
	}

	payloadData := sap.SideAuxBlockTx.Payload.Data(payload.SideChainPowVersion)
	payloadHashData := payloadData[0:32]
	payloadHash, err := common.Uint256FromBytes(payloadHashData)
	if err != nil {
		return false
	}
	if *payloadHash != hashAuxBlock {
		return false
	}

	return true
}
