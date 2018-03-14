package db

import (
	"bytes"
	"io"

	. "SPVWallet/core"
	. "SPVWallet/core/serialization"
	. "SPVWallet/core/auxpow"
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

func (header *Header) Serialize(w io.Writer) error {
	err := WriteUint32(w, header.Version)
	if err != nil {
		return err
	}

	_, err = header.Previous.Serialize(w)
	if err != nil {
		return err
	}

	_, err = header.MerkleRoot.Serialize(w)
	if err != nil {
		return err
	}

	err = WriteUint32(w, header.Timestamp)
	if err != nil {
		return err
	}

	err = WriteUint32(w, header.Bits)
	if err != nil {
		return err
	}

	err = WriteUint32(w, header.Nonce)
	if err != nil {
		return err
	}

	err = WriteUint32(w, header.Height)
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
	var err error
	//Version
	header.Version, err = ReadUint32(r)
	if err != nil {
		return err
	}

	//PrevBlockHash
	err = header.Previous.Deserialize(r)
	if err != nil {
		return err
	}

	//TransactionsRoot
	err = header.MerkleRoot.Deserialize(r)
	if err != nil {
		return err
	}

	//Timestamp
	header.Timestamp, err = ReadUint32(r)
	if err != nil {
		return err
	}

	//Bits
	header.Bits, err = ReadUint32(r)
	if err != nil {
		return err
	}
	//Nonce
	header.Nonce, err = ReadUint32(r)
	if err != nil {
		return err
	}
	//Height
	header.Height, err = ReadUint32(r)
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

func (header *Header) Hash() *Uint256 {
	bytes, _ := header.Bytes()
	hash := Uint256(Sha256D(bytes))
	return &hash
}

func (header *Header) Bytes() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := header.Serialize(buf)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func HeaderFromBytes(headerBytes []byte) (*Header, error) {
	var header Header
	err := header.Deserialize(bytes.NewReader(headerBytes))
	if err != nil {
		return nil, err
	}
	return &header, nil
}
