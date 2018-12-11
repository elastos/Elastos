package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	// MaxTxFilterDataSize is the maximum size in bytes a filter may be.
	MaxTxFilterDataSize = 50000
)

type TxFilterType uint8

const (
	// TFBloom indicates the TxFilter's Filter is a bloom filter.
	TFBloom = iota

	// TFTxType indicates the TxFilter's Filter is a transaction type filter.
	TFTxType
)

// Map of tx filter types back to their constant names for pretty printing.
var tftStrings = map[TxFilterType]string{
	TFBloom:  "TFBloom",
	TFTxType: "TFTxType",
}

// String returns the TxFilterType in human-readable form.
func (f TxFilterType) String() string {
	s, ok := tftStrings[f]
	if ok {
		return s
	}
	return fmt.Sprintf("TFType%d", f)
}

type TxFilterOp uint8

const (
	// OpFilterLoad indicates this TxFilter message is FilterLoad. Which is
	// load a Filter by the given Type.
	OpFilterLoad = iota

	// OpFilterAdd indicates this TxFilter message is FilterAdd. Which is
	// add a Filter by the given Type.
	OpFilterAdd

	// OpFilterClear indicates this TxFilter message is FilterClear. Which is
	// clear a Filter by the given Type.
	OpFilterClear

	// OpClearAll indicates this TxFilter message is ClearAll. Which is
	// clear all filters that already loaded.
	OpClearAll
)

// Map of tx filter ops back to their constant names for pretty printing.
var tfoStrings = map[TxFilterOp]string{
	OpFilterLoad:  "OpFilterLoad",
	OpFilterAdd:   "OpFilterAdd",
	OpFilterClear: "OpFilterClear",
	OpClearAll:    "OpClearAll",
}

// String returns the TxFilterOp in human-readable form.
func (o TxFilterOp) String() string {
	s, ok := tfoStrings[o]
	if ok {
		return s
	}
	return fmt.Sprintf("TFOp%d", o)
}

// Ensure FilterLoad implement p2p.Message interface.
var _ p2p.Message = (*TxFilter)(nil)

// TxFilter is a message to load, add or clear a transaction filter for
// spv usage.
type TxFilter struct {
	Type TxFilterType
	Op   TxFilterOp
	Data []byte
}

func (msg *TxFilter) CMD() string {
	return p2p.CmdTxFilter
}

func (msg *TxFilter) MaxLength() uint32 {
	return 5 + MaxTxFilterDataSize
}

func (msg *TxFilter) Serialize(w io.Writer) error {
	err := common.WriteElements(w, msg.Type, msg.Op)
	if err != nil {
		return err
	}

	return common.WriteVarBytes(w, msg.Data)
}

func (msg *TxFilter) Deserialize(r io.Reader) error {
	err := common.ReadElements(r, &msg.Type, &msg.Op)
	if err != nil {
		return err
	}

	msg.Data, err =  common.ReadVarBytes(r, MaxTxFilterDataSize,
		"txfilter data")
	return err
}
