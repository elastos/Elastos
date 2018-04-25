package core

import (
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const SideMiningPayloadVersion byte = 0x00

type PayloadSideMining struct {
	SideBlockHash   Uint256
	SideGenesisHash Uint256
}

func (a *PayloadSideMining) Data(version byte) []byte {
	data := make([]uint8, 0)
	data = append(data, a.SideBlockHash[:]...)
	data = append(data, a.SideGenesisHash[:]...)

	return data[:]
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
	return nil
}
