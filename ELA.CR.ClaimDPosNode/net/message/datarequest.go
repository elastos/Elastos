package message

import (
	"bytes"
	"encoding/binary"

	"Elastos.ELA/common"
	"Elastos.ELA/common/log"
	"Elastos.ELA/common/serialization"
	. "Elastos.ELA/net/protocol"
)

const (
	TRANSACTION = 0x01
	BLOCK       = 0x02
)

type dataReq struct {
	Header
	reqType uint8
	hash    common.Uint256
}

func ReqBlkData(node Noder, hash common.Uint256) error {
	node.LocalNode().AddRequestedBlock(hash)
	var msg dataReq
	msg.reqType = BLOCK
	msg.hash = hash

	body, err := msg.Serialization()
	if err != nil {
		return err
	}

	buf, err := BuildMessage("getdata", body)
	if err != nil {
		return err
	}

	go node.Tx(buf)

	return nil
}

func (msg dataReq) Handle(node Noder) error {
	log.Debug()
	hash := msg.hash
	switch msg.reqType {
	case BLOCK:
		block, err := NewBlockFromHash(hash)
		if err != nil {
			log.Debug("Can't get block from hash: ", hash, " ,send not found message")
			//call notfound message
			b, err := NewNotFound(hash)
			node.Tx(b)
			return err
		}
		log.Debug("block height is ", block.Blockdata.Height, " ,hash is ", hash)

		var buf []byte
		if node.GetFilter().IsLoaded() {
			buf, err = NewMerkleBlockMsg(block, node.GetFilter())
		} else {
			buf, err = NewBlock(block)
		}
		if err != nil {
			return err
		}
		go node.Tx(buf)

	case TRANSACTION:
		txn, err := NewTxnFromHash(hash)
		if err != nil {
			return err
		}
		buf, err := NewTxn(txn)
		if err != nil {
			return err
		}
		go node.Tx(buf)
	}
	return nil
}

func (msg *dataReq) Deserialization(p []byte) error {
	buf := bytes.NewReader(p)
	err := binary.Read(buf, binary.LittleEndian, &msg.Header)
	if err != nil {
		return err
	}

	msg.reqType, err = serialization.ReadUint8(buf)
	if err != nil {
		return err
	}

	err = msg.hash.Deserialize(buf)
	if err != nil {
		return err
	}
	return nil
}

func (msg dataReq) Serialization() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint8(buf, msg.reqType)
	if err != nil {
		return nil, err
	}

	_, err = msg.hash.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}
