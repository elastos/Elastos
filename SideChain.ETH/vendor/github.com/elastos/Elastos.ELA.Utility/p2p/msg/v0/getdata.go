package v0

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type GetData struct {
	Hash common.Uint256
}

func NewGetData(hash common.Uint256) *GetData {
	return &GetData{Hash: hash}
}

func (msg *GetData) CMD() string {
	return p2p.CmdGetData
}

func (msg *GetData) MaxLength() uint32 {
	return common.UINT256SIZE
}

func (msg *GetData) Serialize(w io.Writer) error {
	return common.WriteElement(w, msg.Hash)
}

func (msg *GetData) Deserialize(r io.Reader) error {
	return common.ReadElement(r, &msg.Hash)
}
