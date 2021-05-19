package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/auxpow"

	"github.com/elastos/Elastos.ELA/common"
)

type Header struct {
	*types.Header

	ReceiptHash common.Uint256
	Bloom       []byte
}

func (header *Header) Serialize(w io.Writer) error {
	err := header.serializeNoAux(w)
	if err != nil {
		return err
	}

	err = header.SideAuxPow.Serialize(w)
	if err != nil {
		return err
	}

	w.Write([]byte{byte(1)})
	return nil
}

func (header *Header) Deserialize(r io.Reader) error {
	err := common.ReadElements(r,
		&header.Version,
		&header.Previous,
		&header.MerkleRoot,
		&header.Timestamp,
		&header.Bits,
		&header.Nonce,
		&header.Height,
		&header.ReceiptHash,
	)
	if err != nil {
		return err
	}

	header.Bloom, err = common.ReadVarBytes(r, 256, "Bloom")
	if err != nil {
		return err
	}

	// SideAuxPow
	err = header.SideAuxPow.Deserialize(r)
	if err != nil {
		return err
	}

	r.Read(make([]byte, 1))

	return nil
}

func (header *Header) serializeNoAux(w io.Writer) error {
	err := common.WriteElements(w,
		header.Version,
		&header.Previous,
		&header.MerkleRoot,
		header.Timestamp,
		header.Bits,
		header.Nonce,
		header.Height,
		&header.ReceiptHash,
	)
	if err != nil {
		return err
	}
	return common.WriteVarBytes(w, header.Bloom)
}

func (header *Header) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	header.serializeNoAux(buf)
	return common.Sha256D(buf.Bytes())
}


func (header *Header) SetVersion(version uint32) {
	header.Version = version
}

func (header *Header) GetVersion() uint32 {
	return header.Version
}

func (header *Header) SetHeight(height uint32) {
	header.Height = height
}

func (header *Header) GetHeight() uint32 {
	return header.Height
}

func (header *Header) GetBits() uint32 {
	return header.Bits
}

func (header *Header) SetBits(bits uint32) {
	header.Bits = bits
}

func (header *Header) GetAuxPow() *auxpow.SideAuxPow {
	return &header.SideAuxPow
}

func (header *Header) SetAuxPow(sideAuxPow *auxpow.SideAuxPow) {
	header.SideAuxPow = *sideAuxPow
}

func (header *Header) SetPrevious(previous common.Uint256) {
	header.Previous = previous
}

func (header *Header) GetPrevious() common.Uint256 {
	return header.Previous
}

func (header *Header) SetMerkleRoot(root common.Uint256) {
	header.MerkleRoot = root
}

func (header *Header) GetMerkleRoot() common.Uint256 {
	return header.MerkleRoot
}

func (header *Header) SetTimeStamp(timestamp uint32) {
	header.Timestamp = timestamp
}

func (header *Header) GetTimeStamp() uint32 {
	return header.Timestamp
}

func (header *Header) SetNonce(nonce uint32) {
	header.Nonce = nonce
}

func (header *Header) GetNonce() uint32 {
	return header.Nonce
}

func NewHeader() *Header {
	header := &Header{
		Header: &types.Header{},
	}
	return header
}
