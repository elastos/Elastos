package message

import (
	"bytes"
	"encoding/binary"
	"errors"

	chain "github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/net/protocol"

	"github.com/elastos/Elastos.ELA.Utility/core/ledger"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

type block struct {
	Header
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

	if chain.DefaultLedger.BlockInLedger(hash) {
		ReceiveDuplicateBlockCnt++
		log.Trace("Receive ", ReceiveDuplicateBlockCnt, " duplicated block.")
		return nil
	}

	chain.DefaultLedger.Store.RemoveHeaderListElement(hash)
	node.LocalNode().DeleteRequestedBlock(hash)
	isOrphan := false
	var err error
	_, isOrphan, err = chain.DefaultLedger.Blockchain.AddBlock(&msg.blk)

	if err != nil {
		log.Warn("Block add failed: ", err, " ,block hash is ", hash.Bytes())
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
			orphanRoot := chain.DefaultLedger.Blockchain.GetOrphanRoot(&hash)
			locator, _ := chain.DefaultLedger.Blockchain.LatestBlockLocator()
			SendMsgSyncBlockHeaders(node, locator, *orphanRoot)
		}
	}

	return nil
}

func NewBlockFromHash(hash common.Uint256) (*ledger.Block, error) {
	bk, err := chain.DefaultLedger.Store.GetBlock(hash)
	if err != nil {
		log.Errorf("Get Block error: %s, block hash: %x", err.Error(), hash)
		return nil, err
	}
	return bk, nil
}

func NewBlock(bk *ledger.Block) ([]byte, error) {
	log.Debug()
	var msg block
	msg.blk = *bk

	body, err := msg.Serialize()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}

	return BuildMessage("block", body)
}

func (msg block) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := msg.blk.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *block) Deserialize(p []byte) error {
	buf := bytes.NewBuffer(p)

	err := binary.Read(buf, binary.LittleEndian, &(msg.Header))
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
