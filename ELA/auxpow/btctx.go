package auxpow

import (
	"bytes"
	"encoding/binary"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	// MaxScriptSize is the maximum allowed length of a raw script.
	MaxScriptSize = 10000
)

type BtcOutPoint struct {
	Hash  common.Uint256
	Index uint32
}

type BtcTxIn struct {
	PreviousOutPoint BtcOutPoint
	SignatureScript  []byte
	Sequence         uint32
}

type BtcTxOut struct {
	Value    int64
	PkScript []byte
}

type BtcTx struct {
	Version  int32
	TxIn     []*BtcTxIn
	TxOut    []*BtcTxOut
	LockTime uint32
}

func BtcReadOutPoint(r io.Reader, op *BtcOutPoint) error {
	_, err := io.ReadFull(r, op.Hash[:])
	if err != nil {
		return err
	}

	var buf [4]byte
	_, err = io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	op.Index = binary.LittleEndian.Uint32(buf[:])
	return nil
}

func BtcWriteOutPoint(w io.Writer, op *BtcOutPoint) error {
	_, err := w.Write(op.Hash[:])
	if err != nil {
		return err
	}

	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], op.Index)
	_, err = w.Write(buf[:])
	return err
}

func BtcReadTxIn(r io.Reader, ti *BtcTxIn) error {
	var op BtcOutPoint
	err := BtcReadOutPoint(r, &op)
	if err != nil {
		return err
	}
	ti.PreviousOutPoint = op

	ti.SignatureScript, err = common.ReadVarBytes(r, MaxScriptSize,
		"transaction input signature script")
	if err != nil {
		return err
	}

	var buf [4]byte
	_, err = io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	ti.Sequence = binary.LittleEndian.Uint32(buf[:])

	return nil
}

func BtcWriteTxIn(w io.Writer, ti *BtcTxIn) error {
	err := BtcWriteOutPoint(w, &ti.PreviousOutPoint)
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, ti.SignatureScript)
	if err != nil {
		return err
	}

	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], ti.Sequence)
	_, err = w.Write(buf[:])
	return err
}

func BtcReadTxOut(r io.Reader, to *BtcTxOut) error {
	var buf [8]byte
	_, err := io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	to.Value = int64(binary.LittleEndian.Uint64(buf[:]))

	to.PkScript, err = common.ReadVarBytes(r, MaxScriptSize,
		"transaction output public key script")
	return err
}

func BtcWriteTxOut(w io.Writer, to *BtcTxOut) error {
	var buf [8]byte
	binary.LittleEndian.PutUint64(buf[:], uint64(to.Value))
	_, err := w.Write(buf[:])
	if err != nil {
		return err
	}

	err = common.WriteVarBytes(w, to.PkScript)
	return err
}

func (tx *BtcTx) Serialize(w io.Writer) error {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], uint32(tx.Version))
	_, err := w.Write(buf[:])
	if err != nil {
		return err
	}
	count := uint64(len(tx.TxIn))
	err = common.WriteVarUint(w, count)
	if err != nil {
		return err
	}
	for _, ti := range tx.TxIn {
		err = BtcWriteTxIn(w, ti)
		if err != nil {
			return err
		}
	}

	count = uint64(len(tx.TxOut))
	err = common.WriteVarUint(w, count)
	if err != nil {
		return err
	}

	for _, to := range tx.TxOut {
		err = BtcWriteTxOut(w, to)
		if err != nil {
			return err
		}
	}
	binary.LittleEndian.PutUint32(buf[:], tx.LockTime)
	_, err = w.Write(buf[:])
	return err
}

func (tx *BtcTx) Deserialize(r io.Reader) error {
	var buf [4]byte
	_, err := io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	tx.Version = int32(binary.LittleEndian.Uint32(buf[:]))

	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	tx.TxIn = make([]*BtcTxIn, 0)
	for i := uint64(0); i < count; i++ {
		var txIn BtcTxIn
		err = BtcReadTxIn(r, &txIn)
		if err != nil {
			return err
		}
		tx.TxIn = append(tx.TxIn, &txIn)
	}

	count, err = common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	tx.TxOut = make([]*BtcTxOut, 0)
	for i := uint64(0); i < count; i++ {
		var txOut BtcTxOut
		err = BtcReadTxOut(r, &txOut)
		if err != nil {
			return err
		}
		tx.TxOut = append(tx.TxOut, &txOut)
	}

	_, err = io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	tx.LockTime = binary.LittleEndian.Uint32(buf[:])

	return nil
}

func (tx *BtcTx) Hash() common.Uint256 {
	buf := new(bytes.Buffer)
	tx.Serialize(buf)
	return common.Sha256D(buf.Bytes())
}

func NewBtcTx(txIn []*BtcTxIn, txOut []*BtcTxOut) *BtcTx {
	tx := &BtcTx{
		Version:  1,
		TxIn:     make([]*BtcTxIn, 0),
		TxOut:    make([]*BtcTxOut, 0),
		LockTime: 0,
	}

	tx.TxIn = append(tx.TxIn, txIn...)
	tx.TxOut = append(tx.TxOut, txOut...)

	return tx
}
