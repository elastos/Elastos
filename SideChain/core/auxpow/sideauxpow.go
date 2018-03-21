package auxpow

import (
	"io"

	. "Elastos.ELA.SideChain/common"
	"Elastos.ELA.SideChain/common/serialization"
)

type SideAuxPow struct {
	SideAuxMerkleBranch []Uint256
	SideAuxMerkleIndex  uint32
	SideAuxBlockTx      ElaTx
	MainBlockHeader     ElaBlockHeader
	AuxPow              AuxPow
}

func NewSideAuxPow(sideAuxMerkleBranch []Uint256,
	sideAuxMerkleIndex uint32,
	sideAuxBlockTx ElaTx,
	mainBlockHeader ElaBlockHeader) *SideAuxPow {

	return &SideAuxPow{
		SideAuxMerkleBranch: sideAuxMerkleBranch,
		SideAuxMerkleIndex:  sideAuxMerkleIndex,
		SideAuxBlockTx:      sideAuxBlockTx,
		MainBlockHeader:     mainBlockHeader,
	}
}

func (ap *SideAuxPow) Serialize(w io.Writer) error {
	err := ap.SideAuxBlockTx.Serialize(w)
	if err != nil {
		return err
	}

	count := uint64(len(ap.SideAuxMerkleBranch))
	err = serialization.WriteVarUint(w, count)
	if err != nil {
		return err
	}

	for _, pcbm := range ap.SideAuxMerkleBranch {
		_, err = pcbm.Serialize(w)
		if err != nil {
			return err
		}
	}
	idx := uint32(ap.SideAuxMerkleIndex)
	err = serialization.WriteUint32(w, idx)
	if err != nil {
		return err
	}

	err = ap.MainBlockHeader.Serialize(w)
	if err != nil {
		return err
	}
	return nil
}

func (ap *SideAuxPow) Deserialize(r io.Reader) error {
	err := ap.SideAuxBlockTx.Deserialize(r)
	if err != nil {
		return err
	}

	count, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	ap.SideAuxMerkleBranch = make([]Uint256, count)
	for i := uint64(0); i < count; i++ {
		temp := Uint256{}
		err = temp.Deserialize(r)
		if err != nil {
			return err
		}
		ap.SideAuxMerkleBranch[i] = temp

	}

	temp, err := serialization.ReadUint32(r)
	if err != nil {
		return err
	}
	ap.SideAuxMerkleIndex = temp

	err = ap.MainBlockHeader.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}
