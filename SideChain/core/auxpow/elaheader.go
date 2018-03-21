package auxpow

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"io"

	. "Elastos.ELA.SideChain/common"
	"Elastos.ELA.SideChain/common/serialization"
)

type ElaBlockHeader struct {
	Version          uint32
	PrevBlockHash    Uint256
	TransactionsRoot Uint256
	Timestamp        uint32
	Bits             uint32
	Height           uint32
	Nonce            uint32
	AuxPow           AuxPow
}

//Serialize the blockheader
func (bd *ElaBlockHeader) Serialize(w io.Writer) {
	bd.SerializeUnsigned(w)
	bd.AuxPow.Serialize(w)
	w.Write([]byte{byte(1)})
}

//Serialize the blockheader data without program
func (bd *ElaBlockHeader) SerializeUnsigned(w io.Writer) error {
	//REVD: implement blockheader SerializeUnsigned
	serialization.WriteUint32(w, bd.Version)
	bd.PrevBlockHash.Serialize(w)
	bd.TransactionsRoot.Serialize(w)
	serialization.WriteUint32(w, bd.Timestamp)
	serialization.WriteUint32(w, bd.Bits)
	serialization.WriteUint32(w, bd.Nonce)
	serialization.WriteUint32(w, bd.Height)

	return nil
}

func (bd *ElaBlockHeader) Deserialize(r io.Reader) error {
	//REVD：Blockdata Deserialize
	bd.DeserializeUnsigned(r)
	bd.AuxPow.Deserialize(r)
	p := make([]byte, 1)
	n, _ := r.Read(p)
	if n > 0 {
		x := []byte(p[:])
		if x[0] != byte(1) {
			return errors.New("ElaBlockHeader Deserialize get format error.")
		}
	} else {
		return errors.New("ElaBlockHeader Deserialize get format error.")
	}

	return nil
}

func (bd *ElaBlockHeader) DeserializeUnsigned(r io.Reader) error {
	//Version
	temp, err := serialization.ReadUint32(r)
	if err != nil {
		return errors.New("ElaBlockHeader item Version Deserialize failed.")
	}
	bd.Version = temp

	//PrevBlockHash
	preBlock := new(Uint256)
	err = preBlock.Deserialize(r)
	if err != nil {
		return errors.New("ElaBlockHeader item preBlock Deserialize failed.")
	}
	bd.PrevBlockHash = *preBlock

	//TransactionsRoot
	txRoot := new(Uint256)
	err = txRoot.Deserialize(r)
	if err != nil {
		return err
	}
	bd.TransactionsRoot = *txRoot

	//Timestamp
	temp, _ = serialization.ReadUint32(r)
	bd.Timestamp = uint32(temp)

	//Bits
	temp, _ = serialization.ReadUint32(r)
	bd.Bits = uint32(temp)

	//Nonce
	temp, _ = serialization.ReadUint32(r)
	bd.Nonce = uint32(temp)

	//Height
	temp, _ = serialization.ReadUint32(r)
	bd.Height = uint32(temp)

	return nil
}

func (bd *ElaBlockHeader) Hash() Uint256 {
	buf := new(bytes.Buffer)
	bd.SerializeUnsigned(buf)

	temp := sha256.Sum256(buf.Bytes())
	f := sha256.Sum256(temp[:])
	hash := Uint256(f)

	return hash
}
