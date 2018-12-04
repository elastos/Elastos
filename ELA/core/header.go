package core

import (
	"bytes"
	"io"

	"github.com/elastos/Elastos.ELA/auxpow"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type Header struct {
	Version    uint32
	Previous   Uint256
	MerkleRoot Uint256
	Timestamp  uint32
	Bits       uint32
	Nonce      uint32
	Height     uint32
	AuxPow     auxpow.AuxPow
}

func (header *Header) Serialize(w io.Writer) error {
	err := header.serializeNoAux(w)
	if err != nil {
		return err
	}

	err = header.AuxPow.Serialize(w)
	if err != nil {
		return err
	}

	w.Write([]byte{byte(1)})
	return nil
}

func (header *Header) Deserialize(r io.Reader) error {
	err := ReadElements(r,
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

	// AuxPow
	err = header.AuxPow.Deserialize(r)
	if err != nil {
		return err
	}

	r.Read(make([]byte, 1))

	return nil
}

func (header *Header) serializeNoAux(w io.Writer) error {
	return WriteElements(w,
		header.Version,
		&header.Previous,
		&header.MerkleRoot,
		header.Timestamp,
		header.Bits,
		header.Nonce,
		header.Height,
	)
}

func (header *Header) Hash() Uint256 {
	buf := new(bytes.Buffer)
	header.serializeNoAux(buf)
	return Uint256(Sha256D(buf.Bytes()))
}
