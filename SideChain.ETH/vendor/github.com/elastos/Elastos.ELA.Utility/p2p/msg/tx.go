package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type Tx struct {
	Transaction common.Serializable
}

func NewTx(tx common.Serializable) *Tx {
	return &Tx{Transaction: tx}
}

func (msg *Tx) CMD() string {
	return p2p.CmdTx
}

func (msg *Tx) MaxLength() uint32 {
	return MaxBlockSize
}

func (msg *Tx) Serialize(writer io.Writer) error {
	return msg.Transaction.Serialize(writer)
}

func (msg *Tx) Deserialize(reader io.Reader) error {
	return msg.Transaction.Deserialize(reader)
}
