package message

import (
	"Elastos.ELA/common"
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/ledger"
	. "Elastos.ELA/net/protocol"
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
)

type block struct {
	messageHeader
	blk ledger.Block
	// TBD
	//event *events.Event
}

func (msg block) Handle(node Noder) error {
	hash := msg.blk.Hash()
	//node.LocalNode().AcqSyncBlkReqSem()
	//defer node.LocalNode().RelSyncBlkReqSem()
	//log.Tracef("hash is %x", hash.ToArrayReverse())
	if node.LocalNode().IsNeighborNoder(node) == false {
		log.Trace("received headers message from unknown peer")
		return errors.New("received headers message from unknown peer")
	}

	if ledger.DefaultLedger.BlockInLedger(hash) {
		ReceiveDuplicateBlockCnt++
		log.Trace("Receive ", ReceiveDuplicateBlockCnt, " duplicated block.")
		return nil
	}

	ledger.DefaultLedger.Store.RemoveHeaderListElement(hash)
	node.LocalNode().DeleteRequestedBlock(hash)
	isOrphan := false
	var err error
	_, isOrphan, err = ledger.DefaultLedger.Blockchain.AddBlock(&msg.blk)

	if err != nil {
		log.Warn("Block add failed: ", err, " ,block hash is ", hash.ToArrayReverse())
		return err
	}
	//relay
	if node.LocalNode().IsSyncHeaders() == false {
		if !node.LocalNode().ExistedID(hash) {
			node.LocalNode().Relay(node, &msg.blk)
			log.Debug("Relay block")
		}
	}

	if isOrphan == true && node.LocalNode().IsSyncHeaders() == false {
		if !node.LocalNode().RequestedBlockExisted(hash) {
			orphanRoot := ledger.DefaultLedger.Blockchain.GetOrphanRoot(&hash)
			locator, _ := ledger.DefaultLedger.Blockchain.LatestBlockLocator()
			SendMsgSyncBlockHeaders(node, locator, *orphanRoot)
		}
	}

	return nil
}

func ReqBlkData(node Noder, hash common.Uint256) error {
	node.LocalNode().AddRequestedBlock(hash)
	var msg dataReq
	msg.hash = hash
	msg.messageHeader.Magic = config.Parameters.Magic
	copy(msg.messageHeader.CMD[0:7], "getdata")
	p := bytes.NewBuffer([]byte{})
	msg.hash.Serialize(p)
	s := sha256.Sum256(p.Bytes())
	s2 := s[:]
	s = sha256.Sum256(s2)
	buf := bytes.NewBuffer(s[:4])
	binary.Read(buf, binary.LittleEndian, &(msg.messageHeader.Checksum))
	msg.messageHeader.Length = uint32(len(p.Bytes()))
	log.Debug("The message payload length is ", msg.messageHeader.Length)

	sendBuf, err := msg.Serialization()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return err
	}

	node.Tx(sendBuf)

	return nil
}

func (msg block) Serialization() ([]byte, error) {
	hdrBuf, err := msg.messageHeader.Serialization()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	msg.blk.Serialize(buf)

	return buf.Bytes(), err
}

func (msg *block) Deserialization(p []byte) error {
	buf := bytes.NewBuffer(p)

	err := binary.Read(buf, binary.LittleEndian, &(msg.messageHeader))
	if err != nil {
		log.Warn("Parse block message hdr error")
		return errors.New("Parse block message hdr error")
	}

	err = msg.blk.Deserialize(buf)
	if err != nil {
		log.Warn("Parse block message error")
		return errors.New("Parse block message error")
	}

	return err
}
