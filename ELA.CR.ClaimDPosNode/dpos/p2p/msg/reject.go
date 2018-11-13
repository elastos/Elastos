package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

// Ensure Reject implement p2p.Message interface.
var _ p2p.Message = (*Reject)(nil)

// RejectCode represents a numeric value by which a remote peer indicates
// why a message was rejected.
type RejectCode uint8

// maxRejectMessageSize is the maximum bytes a reject message.
const maxRejectMessageSize = 1024 * 512 // 512KB

// These constants define the various supported reject codes.
const (
	RejectMalformed       RejectCode = 0x01
	RejectInvalid         RejectCode = 0x10
	RejectObsolete        RejectCode = 0x11
	RejectDuplicate       RejectCode = 0x12
	RejectNonstandard     RejectCode = 0x40
	RejectDust            RejectCode = 0x41
	RejectInsufficientFee RejectCode = 0x42
	RejectCheckpoint      RejectCode = 0x43
)

// Map of reject codes back strings for pretty printing.
var rejectCodeStrings = map[RejectCode]string{
	RejectMalformed:       "REJECT_MALFORMED",
	RejectInvalid:         "REJECT_INVALID",
	RejectObsolete:        "REJECT_OBSOLETE",
	RejectDuplicate:       "REJECT_DUPLICATE",
	RejectNonstandard:     "REJECT_NONSTANDARD",
	RejectDust:            "REJECT_DUST",
	RejectInsufficientFee: "REJECT_INSUFFICIENTFEE",
	RejectCheckpoint:      "REJECT_CHECKPOINT",
}

// String returns the RejectCode in human-readable form.
func (code RejectCode) String() string {
	if s, ok := rejectCodeStrings[code]; ok {
		return s
	}

	return fmt.Sprintf("Unknown RejectCode (%d)", uint8(code))
}

type Reject struct {
	// Cmd is the command for the message which was rejected such as
	// as CmdBlock or CmdReject.  This can be obtained from the Command function
	// of a Message.
	Cmd string

	// RejectCode is a code indicating why the command was rejected.  It
	// is encoded as a uint8 on the wire.
	Code RejectCode

	// Reason is a human-readable string with specific details (over and
	// above the reject code) about why the command was rejected.
	Reason string

	// Hash identifies a specific block or transaction that was rejected
	// and therefore only applies the MsgBlock and MsgReject messages.
	Hash common.Uint256
}

func NewReject(cmd string, code RejectCode, reason string) *Reject {
	return &Reject{
		Cmd:    cmd,
		Code:   code,
		Reason: reason,
	}
}

func (msg *Reject) CMD() string {
	return p2p.CmdReject
}

func (msg *Reject) MaxLength() uint32 {
	return maxRejectMessageSize
}

func (msg *Reject) Serialize(w io.Writer) error {
	if err := common.WriteVarString(w, msg.Cmd); err != nil {
		return err
	}

	if err := common.WriteUint8(w, uint8(msg.Code)); err != nil {
		return err
	}

	if err := common.WriteVarString(w, msg.Reason); err != nil {
		return err
	}

	return msg.Hash.Serialize(w)
}

func (msg *Reject) Deserialize(r io.Reader) (err error) {
	msg.Cmd, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	code, err := common.ReadUint8(r)
	if err != nil {
		return err
	}
	msg.Code = RejectCode(code)

	msg.Reason, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	return msg.Hash.Deserialize(r)
}
