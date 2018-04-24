package auxpow

import (
	"io"

	"github.com/elastos/Elastos.ELA/auxpow"

	. "github.com/elastos/Elastos.ELA/core"
	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type SideAuxPow struct {
	SideAuxMerkleBranch []Uint256
	SideAuxMerkleIndex  int
	SideAuxBlockTx      Transaction
	MainBlockHeader     Header
}

func NewSideAuxPow(sideAuxMerkleBranch []Uint256,
	sideAuxMerkleIndex int,
	sideAuxBlockTx Transaction,
	mainBlockHeader Header) *SideAuxPow {

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

	count := uint64(len(sap.SideAuxMerkleBranch))
	err = WriteVarUint(w, count)
	if err != nil {
		return err
	}

	for _, pcbm := range sap.SideAuxMerkleBranch {
		err = pcbm.Serialize(w)
		if err != nil {
			return err
		}
	}
	idx := uint32(sap.SideAuxMerkleIndex)
	err = WriteUint32(w, idx)
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

	count, err := ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	sap.SideAuxMerkleBranch = make([]Uint256, count)
	for i := uint64(0); i < count; i++ {
		temp := Uint256{}
		err = temp.Deserialize(r)
		if err != nil {
			return err
		}
		sap.SideAuxMerkleBranch[i] = temp

	}

	temp, err := ReadUint32(r)
	if err != nil {
		return err
	}
	sap.SideAuxMerkleIndex = int(temp)

	return sap.MainBlockHeader.Deserialize(r)
}

func (sap *SideAuxPow) SideAuxPowCheck(hashAuxBlock Uint256) bool {
	mainBlockHeader := sap.MainBlockHeader
	mainBlockHeaderHash := mainBlockHeader.Hash()
	if !mainBlockHeader.AuxPow.Check(&mainBlockHeaderHash, auxpow.AuxPowChainID) {
		return false
	}

	sideAuxPowMerkleRoot := auxpow.GetMerkleRoot(sap.SideAuxBlockTx.Hash(), sap.SideAuxMerkleBranch, sap.SideAuxMerkleIndex)
	if sideAuxPowMerkleRoot != sap.MainBlockHeader.MerkleRoot {
		return false
	}

	payloadData := sap.SideAuxBlockTx.Payload.Data(SideMiningPayloadVersion)
	payloadHashData := payloadData[0:32]
	payloadHash, err := Uint256FromBytes(payloadHashData)
	if err != nil {
		return false
	}
	if *payloadHash != hashAuxBlock {
		return false
	}

	return true
}
