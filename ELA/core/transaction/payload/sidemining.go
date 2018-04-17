package payload

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const SideMiningPayloadVersion byte = 0x00

type SideMining struct {
	SideBlockHash   common.Uint256
	SideGenesisHash common.Uint256
}

func (a *SideMining) Data(version byte) []byte {
	data := make([]uint8, 0)
	data = append(data, a.SideBlockHash[:]...)
	data = append(data, a.SideGenesisHash[:]...)

	return data[:]
}

func (a *SideMining) Serialize(w io.Writer, version byte) error {
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

func (a *SideMining) Deserialize(r io.Reader, version byte) error {
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
