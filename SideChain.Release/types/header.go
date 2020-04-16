package types

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA.SideChain/auxpow"

	"github.com/elastos/Elastos.ELA/common"
)

type BaseHeader struct {
	Version    uint32
	Previous   common.Uint256
	MerkleRoot common.Uint256
	Timestamp  uint32
	Bits       uint32
	Nonce      uint32
	Height     uint32
}

func (header *BaseHeader) serializeNoAux(w io.Writer) error {
	return common.WriteElements(w,
		header.Version,
		&header.Previous,
		&header.MerkleRoot,
		header.Timestamp,
		header.Bits,
		header.Nonce,
		header.Height,
	)
}

func (header *BaseHeader) Serialize(w io.Writer) error {
	err := header.serializeNoAux(w)
	if err != nil {
		return err
	}

	w.Write([]byte{byte(1)})
	return nil
}

func (header *BaseHeader) Deserialize(r io.Reader) error {
	err := common.ReadElements(r,
		&header.Version,
		&header.Previous,
		&header.MerkleRoot,
		&header.Timestamp,
		&header.Bits,
		&header.Nonce,
		&header.Height,
	)
	if err != nil {
		return err
	}
	return nil
}

type Header struct {
	Base       BaseHeader
	SideAuxPow auxpow.SideAuxPow
}

func (header *Header) SetVersion(version uint32) {
	header.Base.Version = version
}

func (header *Header) GetVersion() uint32 {
	return header.Base.Version
}

func (header *Header) Serialize(w io.Writer) error {
	err := header.Base.serializeNoAux(w)
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
	err := header.Base.Deserialize(r)
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

func (header *Header) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	header.Base.serializeNoAux(buf)
	return common.Sha256D(buf.Bytes())
}

func (header *Header) SetHeight(height uint32) {
	header.Base.Height = height
}

func (header *Header) GetHeight() uint32 {
	return header.Base.Height
}

func (header *Header) GetBits() uint32 {
	return header.Base.Bits
}

func (header *Header) SetBits(bits uint32) {
	header.Base.Bits = bits
}

func (header *Header) GetAuxPow() *auxpow.SideAuxPow {
	return &header.SideAuxPow
}

func (header *Header) SetAuxPow(sideAuxPow *auxpow.SideAuxPow) {
	header.SideAuxPow = *sideAuxPow
}

func (header *Header) SetPrevious(previous common.Uint256) {
	header.Base.Previous = previous
}

func (header *Header) GetPrevious() common.Uint256 {
	return header.Base.Previous
}

func (header *Header) SetMerkleRoot(root common.Uint256) {
	header.Base.MerkleRoot = root
}

func (header *Header) GetMerkleRoot() common.Uint256 {
	return header.Base.MerkleRoot
}

func (header *Header) SetTimeStamp(timestamp uint32) {
	header.Base.Timestamp = timestamp
}

func (header *Header) GetTimeStamp() uint32 {
	return header.Base.Timestamp
}

func (header *Header) SetNonce(nonce uint32) {
	header.Base.Nonce = nonce
}

func (header *Header) GetNonce() uint32 {
	return header.Base.Nonce
}
