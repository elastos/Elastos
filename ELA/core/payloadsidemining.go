package core

import (
	"bytes"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const SideMiningPayloadVersion byte = 0x00

type PayloadSideMining struct {
	SideBlockHash   Uint256
	SideGenesisHash Uint256
	BlockHeight     uint64
}

func (a *PayloadSideMining) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}

	return buf.Bytes()
}

func (a *PayloadSideMining) Serialize(w io.Writer, version byte) error {
	err := a.SideBlockHash.Serialize(w)
	if err != nil {
		return err
	}
	err = a.SideGenesisHash.Serialize(w)
	if err != nil {
		return err
	}
	err = WriteUint64(w, a.BlockHeight)
	if err != nil {
		return err
	}
	return nil
}

func (a *PayloadSideMining) Deserialize(r io.Reader, version byte) error {
	err := a.SideBlockHash.Deserialize(r)
	if err != nil {
		return err
	}
	err = a.SideGenesisHash.Deserialize(r)
	if err != nil {
		return err
	}
	height, err := ReadUint64(r)
	if err != nil {
		return err
	}
	a.BlockHeight = height
	return nil
}
