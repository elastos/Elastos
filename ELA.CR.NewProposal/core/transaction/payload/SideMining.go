package payload

import (
	"io"

	"Elastos.ELA/common"
)

const SideMiningPayloadVersion byte = 0x00

type SideMining struct {
	SideBlockHash common.Uint256
}

func (a *SideMining) Data(version byte) []byte {
	return a.SideBlockHash[:]
}

func (a *SideMining) Serialize(w io.Writer, version byte) error {
	_, err := a.SideBlockHash.Serialize(w)
	return err
}

func (a *SideMining) Deserialize(r io.Reader, version byte) error {
	err := a.SideBlockHash.Deserialize(r)
	return err
}
