package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p"
)

const (
	// MaxTxFilterLoadDataSize is the maximum size in bytes a filter may be.
	MaxTxFilterLoadDataSize = 50000
)

// Ensure FilterLoad implement p2p.Message interface.
var _ p2p.Message = (*TxFilterLoad)(nil)

// TxFilterLoad is a message to load a transaction filter, it can be a bloom
// filter or other transaction filters.
type TxFilterLoad struct {
	Type uint8
	Data []byte
}

func (msg *TxFilterLoad) CMD() string {
	return p2p.CmdTxFilter
}

func (msg *TxFilterLoad) MaxLength() uint32 {
	return 4 + MaxTxFilterLoadDataSize
}

func (msg *TxFilterLoad) Serialize(w io.Writer) error {
	err := common.WriteElements(w, msg.Type)
	if err != nil {
		return err
	}

	return common.WriteVarBytes(w, msg.Data)
}

func (msg *TxFilterLoad) Deserialize(r io.Reader) error {
	err := common.ReadElements(r, &msg.Type)
	if err != nil {
		return err
	}

	msg.Data, err = common.ReadVarBytes(r, MaxTxFilterLoadDataSize,
		"txfilterload data")
	return err
}
