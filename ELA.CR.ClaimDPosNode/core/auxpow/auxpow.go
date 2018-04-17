package auxpow

import (
	"encoding/binary"
	"encoding/hex"
	"io"
	"strings"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/common/serialize"
)

var (
	AuxPowChainID         = 1
	pchMergedMiningHeader = []byte{0xfa, 0xbe, 'm', 'm'}
)

type AuxPow struct {
	AuxMerkleBranch   []Uint256
	AuxMerkleIndex    int
	ParCoinbaseTx     BtcTx
	ParCoinBaseMerkle []Uint256
	ParMerkleIndex    int
	ParBlockHeader    BtcHeader
	ParentHash        Uint256
}

func NewAuxPow(AuxMerkleBranch []Uint256, AuxMerkleIndex int,
	ParCoinbaseTx BtcTx, ParCoinBaseMerkle []Uint256,
	ParMerkleIndex int, ParBlockHeader BtcHeader) *AuxPow {

	return &AuxPow{
		AuxMerkleBranch:   AuxMerkleBranch,
		AuxMerkleIndex:    AuxMerkleIndex,
		ParCoinbaseTx:     ParCoinbaseTx,
		ParCoinBaseMerkle: ParCoinBaseMerkle,
		ParMerkleIndex:    ParMerkleIndex,
		ParBlockHeader:    ParBlockHeader,
	}
}

func (ap *AuxPow) Serialize(w io.Writer) error {
	err := ap.ParCoinbaseTx.Serialize(w)
	if err != nil {
		return err
	}

	err = ap.ParentHash.Serialize(w)
	if err != nil {
		return err
	}

	idx := uint32(ap.AuxMerkleIndex)
	err = serialize.WriteUint32(w, idx)
	if err != nil {
		return err
	}

	count := uint64(len(ap.AuxMerkleBranch))
	err = serialize.WriteVarUint(w, count)
	if err != nil {
		return err
	}

	for _, amb := range ap.AuxMerkleBranch {
		err = amb.Serialize(w)
		if err != nil {
			return err
		}
	}

	idx = uint32(ap.ParMerkleIndex)
	err = serialize.WriteUint32(w, idx)
	if err != nil {
		return err
	}

	count = uint64(len(ap.ParCoinBaseMerkle))
	err = serialize.WriteVarUint(w, count)
	if err != nil {
		return err
	}

	for _, pcbm := range ap.ParCoinBaseMerkle {
		err = pcbm.Serialize(w)
		if err != nil {
			return err
		}
	}

	err = ap.ParBlockHeader.Serialize(w)
	if err != nil {
		return err
	}
	return nil
}

func (ap *AuxPow) Deserialize(r io.Reader) error {
	err := ap.ParCoinbaseTx.Deserialize(r)
	if err != nil {
		return err
	}

	err = ap.ParentHash.Deserialize(r)
	if err != nil {
		return err
	}

	temp, err := serialize.ReadUint32(r)
	if err != nil {
		return err
	}
	ap.AuxMerkleIndex = int(temp)

	count, err := serialize.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	ap.AuxMerkleBranch = make([]Uint256, count)
	for i := uint64(0); i < count; i++ {
		temp := Uint256{}
		err = temp.Deserialize(r)
		if err != nil {
			return err
		}
		ap.AuxMerkleBranch[i] = temp
	}

	temp, err = serialize.ReadUint32(r)
	if err != nil {
		return err
	}
	ap.ParMerkleIndex = int(temp)

	count, err = serialize.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	ap.ParCoinBaseMerkle = make([]Uint256, count)
	for i := uint64(0); i < count; i++ {
		temp := Uint256{}
		err = temp.Deserialize(r)
		if err != nil {
			return err
		}
		ap.ParCoinBaseMerkle[i] = temp

	}

	err = ap.ParBlockHeader.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}

func (ap *AuxPow) Check(hashAuxBlock *Uint256, chainId int) bool {
	if GetMerkleRoot(ap.ParCoinbaseTx.Hash(), ap.ParCoinBaseMerkle, ap.ParMerkleIndex) != ap.ParBlockHeader.MerkleRoot {
		return false
	}

	if len(ap.AuxMerkleBranch) > 0 {
		hashAuxBlockBytes := BytesReverse(hashAuxBlock.Bytes())
		hashAuxBlock, _ = Uint256FromBytes(hashAuxBlockBytes)
	}

	auxRootHashReverse := GetMerkleRoot(*hashAuxBlock, ap.AuxMerkleBranch, ap.AuxMerkleIndex)

	script := ap.ParCoinbaseTx.TxIn[0].SignatureScript
	scriptStr := hex.EncodeToString(script)
	//fixme reverse
	auxRootHashStr := hex.EncodeToString(auxRootHashReverse.Bytes())
	pchMergedMiningHeaderStr := hex.EncodeToString(pchMergedMiningHeader)

	headerIndex := strings.Index(scriptStr, pchMergedMiningHeaderStr)
	rootHashIndex := strings.Index(scriptStr, auxRootHashStr)

	if (headerIndex == -1) || (rootHashIndex == -1) {
		return false
	}

	if strings.Index(scriptStr[headerIndex+2:], pchMergedMiningHeaderStr) != -1 {
		return false
	}

	if headerIndex+len(pchMergedMiningHeaderStr) != rootHashIndex {
		return false
	}

	rootHashIndex += len(auxRootHashStr)
	if len(scriptStr)-rootHashIndex < 8 {
		return false
	}

	size := binary.LittleEndian.Uint32(script[rootHashIndex/2: rootHashIndex/2+4])
	merkleHeight := len(ap.AuxMerkleBranch)
	if size != uint32(1<<uint32(merkleHeight)) {
		return false
	}

	nonce := binary.LittleEndian.Uint32(script[rootHashIndex/2+4: rootHashIndex/2+8])
	if ap.AuxMerkleIndex != GetExpectedIndex(nonce, chainId, merkleHeight) {
		return false
	}

	return true
}

func GetMerkleRoot(hash Uint256, merkleBranch []Uint256, index int) Uint256 {
	if index == -1 {
		return Uint256{}
	}
	var sha [64]byte
	for _, it := range merkleBranch {
		if (index & 1) == 1 {
			copy(sha[:32], it[:])
			copy(sha[32:], hash[:])
			hash = Uint256(Sha256D(sha[:]))
		} else {
			copy(sha[:32], hash[:])
			copy(sha[32:], it[:])
			hash = Uint256(Sha256D(sha[:]))
		}
		index >>= 1
	}
	return hash
}

func GetExpectedIndex(nonce uint32, chainId, h int) int {
	rand := nonce
	rand = rand*1103515245 + 12345
	rand += uint32(chainId)
	rand = rand*1103515245 + 12345

	return int(rand % (1 << uint32(h)))
}
