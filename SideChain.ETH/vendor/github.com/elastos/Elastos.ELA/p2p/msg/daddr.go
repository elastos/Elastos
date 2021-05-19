package msg

import (
	"bytes"
	"io"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/elastos/Elastos.ELA/p2p"
)

const (
	// maxCipherLength indicates the max length of the address cipher.
	maxCipherLength = 256
)

// Ensure Addr implement p2p.Message interface.
var _ p2p.Message = (*DAddr)(nil)

// DAddr represents a DPOS peer address.
type DAddr struct {
	// The peer ID indicates who's address it is.
	PID [33]byte

	// Timestamp represents the time when this message created.
	Timestamp time.Time

	// Which peer ID is used to encode the address cipher.
	Encode [33]byte

	// The encrypted network address using the encode peer ID.
	Cipher []byte

	// Signature of the encode peer ID and cipher to proof the sender itself.
	Signature []byte
}

func (a *DAddr) CMD() string {
	return p2p.CmdDAddr
}

func (a *DAddr) MaxLength() uint32 {
	return 387 // 33+33+256+65
}

func (a *DAddr) Serialize(w io.Writer) error {
	var timestamp = a.Timestamp.Unix()
	err := common.WriteElements(w, a.PID, timestamp, a.Encode)
	if err != nil {
		return err
	}

	if err := common.WriteVarBytes(w, a.Cipher); err != nil {
		return err
	}

	return common.WriteVarBytes(w, a.Signature)
}

func (a *DAddr) Deserialize(r io.Reader) error {
	var timestamp int64
	err := common.ReadElements(r, &a.PID, &timestamp, &a.Encode)
	if err != nil {
		return err
	}
	a.Timestamp = time.Unix(timestamp, 0)

	a.Cipher, err = common.ReadVarBytes(r, maxCipherLength, "DAddr.Cipher")
	if err != nil {
		return err
	}

	a.Signature, err = common.ReadVarBytes(r, crypto.SignatureLength,
		"DAddr.Signature")
	return err
}

func (a *DAddr) Data() []byte {
	b := new(bytes.Buffer)
	var timestamp = a.Timestamp.Unix()
	common.WriteElements(b, timestamp, a.Encode)
	common.WriteVarBytes(b, a.Cipher)
	return b.Bytes()
}

func (a *DAddr) Hash() common.Uint256 {
	return common.Sha256D(a.Data())
}
