package auxpow

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"io"

	. "github.com/elastos/Elastos.ELA.SPV/core"
	"github.com/elastos/Elastos.ELA.SPV/core/serialization"
)

type BtcOutPoint struct {
	Hash  Uint256
	Index uint32
}

type TxIn struct {
	PreviousOutPoint BtcOutPoint
	SignatureScript  []byte
	Sequence         uint32
}

type TxOut struct {
	Value    int64
	PkScript []byte
}

type Tx struct {
	Version  int32
	TxIn     []*TxIn
	TxOut    []*TxOut
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

func BtcReadTxIn(r io.Reader, ti *TxIn) error {
	var op BtcOutPoint
	err := BtcReadOutPoint(r, &op)
	if err != nil {
		return err
	}
	ti.PreviousOutPoint = op

	ti.SignatureScript, err = serialization.ReadVarBytes(r)
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

func BtcWriteTxIn(w io.Writer, ti *TxIn) error {
	err := BtcWriteOutPoint(w, &ti.PreviousOutPoint)
	if err != nil {
		return err
	}

	err = serialization.WriteVarBytes(w, ti.SignatureScript)
	if err != nil {
		return err
	}

	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], ti.Sequence)
	_, err = w.Write(buf[:])
	return err
}

func BtcReadTxOut(r io.Reader, to *TxOut) error {
	var buf [8]byte
	_, err := io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	to.Value = int64(binary.LittleEndian.Uint64(buf[:]))

	to.PkScript, err = serialization.ReadVarBytes(r)
	return err
}

func BtcWriteTxOut(w io.Writer, to *TxOut) error {
	var buf [8]byte
	binary.LittleEndian.PutUint64(buf[:], uint64(to.Value))
	_, err := w.Write(buf[:])
	if err != nil {
		return err
	}

	err = serialization.WriteVarBytes(w, to.PkScript)
	return err
}

func (tx *Tx) Serialize(w io.Writer) error {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], uint32(tx.Version))
	_, err := w.Write(buf[:])
	if err != nil {
		return err
	}
	count := uint64(len(tx.TxIn))
	err = serialization.WriteVarUint(w, count)
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
	err = serialization.WriteVarUint(w, count)
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

func (tx *Tx) Deserialize(r io.Reader) error {
	var buf [4]byte
	_, err := io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	tx.Version = int32(binary.LittleEndian.Uint32(buf[:]))

	count, err := serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	tx.TxIn = make([]*TxIn, count)
	for i := uint64(0); i < count; i++ {
		ti := TxIn{}
		err = BtcReadTxIn(r, &ti)
		if err != nil {
			return err
		}
		tx.TxIn[i] = &ti
	}

	count, err = serialization.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	tx.TxOut = make([]*TxOut, count)
	for i := uint64(0); i < count; i++ {
		to := TxOut{}
		err = BtcReadTxOut(r, &to)
		if err != nil {
			return err
		}
		tx.TxOut[i] = &to
	}

	_, err = io.ReadFull(r, buf[:])
	if err != nil {
		return err
	}
	tx.LockTime = binary.LittleEndian.Uint32(buf[:])

	return nil
}

func (tx *Tx) Hash() Uint256 {
	b_buf := new(bytes.Buffer)
	tx.Serialize(b_buf)
	temp := sha256.Sum256(b_buf.Bytes())
	return Uint256(sha256.Sum256(temp[:]))
}

func NewBtcTx(txIn []*TxIn, txOut []*TxOut) *Tx {
	tx := &Tx{
		Version:  1,
		TxIn:     make([]*TxIn, 0),
		TxOut:    make([]*TxOut, 0),
		LockTime: 0,
	}

	tx.TxIn = append(tx.TxIn, txIn...)
	tx.TxOut = append(tx.TxOut, txOut...)

	return tx
}
