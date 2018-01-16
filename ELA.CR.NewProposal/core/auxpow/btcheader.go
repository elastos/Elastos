package auxpow

import (
	. "Elastos.ELA/common"
	"Elastos.ELA/common/serialization"
	"bytes"
	"crypto/sha256"
	"io"
)

type BtcBlockHeader struct {
	Version    int32
	PrevBlock  Uint256
	MerkleRoot Uint256
	Timestamp  uint32
	Bits       uint32
	Nonce      uint32
}

func (bh *BtcBlockHeader) Serialize(w io.Writer) error {
	serialization.WriteUint32(w, uint32(bh.Version))
	bh.PrevBlock.Serialize(w)
	bh.MerkleRoot.Serialize(w)
	serialization.WriteUint32(w, bh.Timestamp)
	serialization.WriteUint32(w, bh.Bits)
	serialization.WriteUint32(w, bh.Nonce)
	return nil
}

func (bh *BtcBlockHeader) Deserialize(r io.Reader) error {
	//Version
	temp, err := serialization.ReadUint32(r)
	if err != nil {
		return err
	}
	bh.Version = int32(temp)

	//PrevBlockHash
	preBlock := new(Uint256)
	err = preBlock.Deserialize(r)
	if err != nil {
		return err
	}
	bh.PrevBlock = *preBlock

	//TransactionsRoot
	txRoot := new(Uint256)
	err = txRoot.Deserialize(r)
	if err != nil {
		return err
	}
	bh.MerkleRoot = *txRoot

	//Timestamp
	temp, _ = serialization.ReadUint32(r)
	bh.Timestamp = temp

	//Bits
	temp, _ = serialization.ReadUint32(r)
	bh.Bits = temp

	//Nonce
	bh.Nonce, _ = serialization.ReadUint32(r)

	return nil
}

func (bh *BtcBlockHeader) Hash() Uint256 {
	buf := new(bytes.Buffer)
	bh.Serialize(buf)
	temp := sha256.Sum256(buf.Bytes())
	return Uint256(sha256.Sum256(temp[:]))
}
