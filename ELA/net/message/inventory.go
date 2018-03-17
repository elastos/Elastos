package message

import (
	. "Elastos.ELA/common"
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/common/serialization"
	"Elastos.ELA/core/ledger"
	. "Elastos.ELA/net/protocol"
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"io"
	"time"
)

type InvPayload struct {
	Type uint8
	Cnt  uint32
	Blk  []byte
}

type Inv struct {
	Header
	P InvPayload
}

func SendMsgSyncBlockHeaders(node Noder, blocator []Uint256, hash Uint256) {
	if node.LocalNode().GetStartHash() == blocator[0] &&
		node.LocalNode().GetStopHash() == hash {
		return
	}

	buf, err := NewBlocksReq(blocator, hash)
	if err != nil {
		log.Error("failed build a new getblocksReq")
	} else {
		node.LocalNode().SetSyncHeaders(true)
		node.SetSyncHeaders(true)
		go node.Tx(buf)
		node.LocalNode().SetStartHash(blocator[0])
		node.LocalNode().SetStopHash(hash)
	}
}

func NewBlocksReq(blocator []Uint256, hash Uint256) ([]byte, error) {
	var msg blocksReq
	msg.Magic = config.Parameters.Magic
	cmd := "getblocks"
	copy(msg.CMD[0:len(cmd)], cmd)
	tmpBuffer := bytes.NewBuffer([]byte{})
	msg.p.len = uint32(len(blocator))
	msg.p.hashStart = blocator
	serialization.WriteUint32(tmpBuffer, uint32(msg.p.len))

	for _, hash := range blocator {
		_, err := hash.Serialize(tmpBuffer)
		if err != nil {
			return nil, err
		}
	}

	msg.p.hashEnd = hash

	_, err := msg.p.hashEnd.Serialize(tmpBuffer)
	if err != nil {
		return nil, err
	}
	p := new(bytes.Buffer)
	err = binary.Write(p, binary.LittleEndian, tmpBuffer.Bytes())
	if err != nil {
		log.Error("Binary Write failed at new Msg")
		return nil, err
	}
	s := sha256.Sum256(p.Bytes())
	s2 := s[:]
	s = sha256.Sum256(s2)
	buf := bytes.NewBuffer(s[:4])
	binary.Read(buf, binary.LittleEndian, &(msg.Checksum))
	msg.Length = uint32(len(p.Bytes()))
	log.Debug("The message payload length is ", msg.Length)

	m, err := msg.Serialize()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}

	return m, nil
}

func (msg Inv) Handle(node Noder) error {
	log.Debug()
	var id Uint256
	str := hex.EncodeToString(msg.P.Blk)

	log.Debug(fmt.Sprintf("The inv type: no one. block len: %d, %s\n", len(msg.P.Blk), str))

	log.Debug("RX block message")
	if node.LocalNode().IsSyncHeaders() == true && node.IsSyncHeaders() == false {
		return nil
	}

	//return nil
	var i uint32
	count := msg.P.Cnt
	hashes := []Uint256{}
	for i = 0; i < count; i++ {
		id.Deserialize(bytes.NewReader(msg.P.Blk[HASHLEN*i:]))
		hashes = append(hashes, id)
		if ledger.DefaultLedger.Blockchain.IsKnownOrphan(&id) {
			orphanRoot := ledger.DefaultLedger.Blockchain.GetOrphanRoot(&id)
			locator, err := ledger.DefaultLedger.Blockchain.LatestBlockLocator()
			if err != nil {
				log.Errorf(" Failed to get block "+
					"locator for the latest block: "+
					"%v", err)
				continue
			}
			SendMsgSyncBlockHeaders(node, locator, *orphanRoot)
			continue
		}

		if i == (count - 1) {
			var emptyHash Uint256
			blocator := ledger.DefaultLedger.Blockchain.BlockLocatorFromHash(&id)
			SendMsgSyncBlockHeaders(node, blocator, emptyHash)
		}
	}

	for _, h := range hashes {
		// TODO check the ID queue
		if !ledger.DefaultLedger.BlockInLedger(h) {
			if !(node.LocalNode().RequestedBlockExisted(h) || ledger.DefaultLedger.Blockchain.IsKnownOrphan(&h)) {
				<-time.After(time.Millisecond * 50)
				ReqBlkData(node, h)
			}
		}
	}
	return nil
}

func (msg Inv) Serialize() ([]byte, error) {
	hdrBuf, err := msg.Header.Serialize()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	msg.P.Serialization(buf)

	return buf.Bytes(), err
}

func (msg *Inv) Deserialize(p []byte) error {
	err := msg.Header.Deserialize(p)
	if err != nil {
		return err
	}

	buf := bytes.NewBuffer(p[MSGHDRLEN:])
	if err != nil {
		return err
	}
	msg.P.Cnt, err = serialization.ReadUint32(buf)
	if err != nil {
		return err
	}

	msg.P.Blk = make([]byte, msg.P.Cnt*HASHLEN)
	err = binary.Read(buf, binary.LittleEndian, &(msg.P.Blk))

	return err
}

func NewInv(inv *InvPayload) ([]byte, error) {
	var msg Inv
	msg.P.Type = inv.Type
	msg.P.Blk = inv.Blk
	msg.P.Cnt = inv.Cnt
	msg.Magic = config.Parameters.Magic
	cmd := "inv"
	copy(msg.CMD[0:len(cmd)], cmd)
	tmpBuffer := bytes.NewBuffer([]byte{})
	inv.Serialization(tmpBuffer)

	b := new(bytes.Buffer)
	err := binary.Write(b, binary.LittleEndian, tmpBuffer.Bytes())
	if err != nil {
		log.Error("Binary Write failed at new Msg", err.Error())
		return nil, err
	}
	s := sha256.Sum256(b.Bytes())
	s2 := s[:]
	s = sha256.Sum256(s2)
	buf := bytes.NewBuffer(s[:4])
	binary.Read(buf, binary.LittleEndian, &(msg.Checksum))
	msg.Length = uint32(len(b.Bytes()))

	m, err := msg.Serialize()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}

	return m, nil
}

func (msg *InvPayload) Serialization(w io.Writer) {
	serialization.WriteUint8(w, msg.Type)
	serialization.WriteUint32(w, msg.Cnt)

	binary.Write(w, binary.LittleEndian, msg.Blk)
}
