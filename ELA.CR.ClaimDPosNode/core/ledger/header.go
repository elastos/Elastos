package ledger

import (
	"bytes"
	"io"

	. "github.com/elastos/Elastos.ELA/common"
	. "github.com/elastos/Elastos.ELA/common/serialize"
	. "github.com/elastos/Elastos.ELA/core/auxpow"
)

type Header struct {
	Version    uint32
	Previous   Uint256
	MerkleRoot Uint256
	Timestamp  uint32
	Bits       uint32
	Nonce      uint32
	Height     uint32
	AuxPow     AuxPow
}

func (header *Header) SerializeWithoutAux(w io.Writer) error {
	return WriteElements(w,
		header.Version,
		header.Previous,
		header.MerkleRoot,
		header.Timestamp,
		header.Bits,
		header.Nonce,
		header.Height,
	)
}

func (header *Header) Serialize(w io.Writer) error {
	err := header.SerializeWithoutAux(w)
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

func (header *Header) DeserializeWithoutAux(r io.Reader) error {
	return ReadElements(r,
		&header.Version,
		&header.Previous,
		&header.MerkleRoot,
		&header.Timestamp,
		&header.Bits,
		&header.Nonce,
		&header.Height,
	)
}

func (header *Header) Deserialize(r io.Reader) error {
	err := header.DeserializeWithoutAux(r)
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

func (header *Header) Hash() Uint256 {
	buf := new(bytes.Buffer)
	header.SerializeWithoutAux(buf)
	return Uint256(Sha256D(buf.Bytes()))
}

func (header *Header) Bytes() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := header.Serialize(buf)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}
