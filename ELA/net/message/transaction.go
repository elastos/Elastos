package message

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"

	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/transaction"
	. "github.com/elastos/Elastos.ELA/errors"
	. "github.com/elastos/Elastos.ELA/net/protocol"
)

// Transaction message
type trn struct {
	messageHeader
	// TBD
	//txn []byte
	txn transaction.Transaction
	//hash common.Uint256
}

func (msg trn) Handle(node Noder) error {
	log.Debug()
	log.Debug("RX Transaction message")
	tx := &msg.txn
	if !node.LocalNode().ExistedID(tx.Hash()) {
		if errCode := node.LocalNode().AppendToTxnPool(&(msg.txn)); errCode != Success {
			return errors.New("[message] VerifyTransaction failed when AppendToTxnPool.")
		}
		node.LocalNode().Relay(node, tx)
		log.Info("Relay Transaction")
		node.LocalNode().IncRxTxnCnt()
		log.Debug("RX Transaction message hash", msg.txn.Hash())
		log.Debug("RX Transaction message type", msg.txn.TxType)
	}

	return nil
}

func NewTxn(txn *transaction.Transaction) ([]byte, error) {
	log.Debug()
	var msg trn

	msg.messageHeader.Magic = config.Parameters.Magic
	cmd := "tx"
	copy(msg.messageHeader.CMD[0:len(cmd)], cmd)
	tmpBuffer := bytes.NewBuffer([]byte{})
	txn.Serialize(tmpBuffer)
	msg.txn = *txn
	b := new(bytes.Buffer)
	err := binary.Write(b, binary.LittleEndian, tmpBuffer.Bytes())
	if err != nil {
		log.Error("Binary Write failed at new Msg")
		return nil, err
	}
	s := sha256.Sum256(b.Bytes())
	s2 := s[:]
	s = sha256.Sum256(s2)
	buf := bytes.NewBuffer(s[:4])
	binary.Read(buf, binary.LittleEndian, &(msg.messageHeader.Checksum))
	msg.messageHeader.Length = uint32(len(b.Bytes()))
	log.Debug("The message payload length is ", msg.messageHeader.Length)

	m, err := msg.Serialization()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}

	return m, nil
}

func (msg trn) Serialization() ([]byte, error) {
	hdrBuf, err := msg.messageHeader.Serialization()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	msg.txn.Serialize(buf)

	return buf.Bytes(), err
}

func (msg *trn) Deserialization(p []byte) error {
	buf := bytes.NewBuffer(p)
	err := binary.Read(buf, binary.LittleEndian, &(msg.messageHeader))
	err = msg.txn.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
