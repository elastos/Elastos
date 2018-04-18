package message

import (
	"bytes"
	"encoding/binary"
	"errors"

	"github.com/elastos/Elastos.ELA/log"
	chain "github.com/elastos/Elastos.ELA/blockchain"
	. "github.com/elastos/Elastos.ELA/net/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type blocksReq struct {
	Hdr
	p struct {
		len       uint32
		hashStart []Uint256
		hashEnd   Uint256
	}
}

func (msg blocksReq) Handle(node Noder) error {
	log.Debug()
	// lock
	node.LocalNode().AcqSyncHdrReqSem()
	defer node.LocalNode().RelSyncHdrReqSem()
	var locatorHash []Uint256
	var startHash [HASHLEN]byte
	var stopHash [HASHLEN]byte
	locatorHash = msg.p.hashStart
	stopHash = msg.p.hashEnd
	startHash = chain.DefaultLedger.Blockchain.LatestLocatorHash(locatorHash)
	inv, err := GetInvFromBlockHash(startHash, stopHash)
	if err != nil {
		return err
	}
	buf, err := NewInv(inv)
	if err != nil {
		return err
	}
	go node.Tx(buf)
	return nil
}

func (msg blocksReq) Serialize() ([]byte, error) {
	hdrBuf, err := msg.Hdr.Serialize()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	err = binary.Write(buf, binary.LittleEndian, msg.p.len)
	if err != nil {
		return nil, err
	}
	for _, hash := range msg.p.hashStart {
		hash.Serialize(buf)
	}

	msg.p.hashEnd.Serialize(buf)

	return buf.Bytes(), err
}

func (msg *blocksReq) Deserialize(p []byte) error {
	buf := bytes.NewBuffer(p)
	err := binary.Read(buf, binary.LittleEndian, &(msg.Hdr))
	if err != nil {
		return err
	}

	err = binary.Read(buf, binary.LittleEndian, &(msg.p.len))
	if err != nil {
		return err
	}

	for i := 0; i < int(msg.p.len); i++ {
		var hash Uint256
		err := (&hash).Deserialize(buf)
		msg.p.hashStart = append(msg.p.hashStart, hash)
		if err != nil {
			log.Debug("blkHeader req Deserialization failed")
			goto blksReqErr
		}
	}

	err = msg.p.hashEnd.Deserialize(buf)
blksReqErr:
	return err
}

func GetInvFromBlockHash(startHash Uint256, stopHash Uint256) (*InvPayload, error) {
	var count uint32 = 0
	var empty Uint256
	var startHeight uint32
	var stopHeight uint32
	curHeight := chain.DefaultLedger.Store.GetHeight()
	if stopHash == empty {
		if startHash == empty {
			if curHeight > MAXINVHDRCNT {
				count = MAXINVHDRCNT
			} else {
				count = curHeight
			}
		} else {
			bkstart, err := chain.DefaultLedger.Store.GetHeader(startHash)
			if err != nil {
				return nil, err
			}
			startHeight = bkstart.Height
			count = curHeight - startHeight
			if count > MAXINVHDRCNT {
				count = MAXINVHDRCNT
			}
		}
	} else {
		bkstop, err := chain.DefaultLedger.Store.GetHeader(stopHash)
		if err != nil {
			return nil, err
		}
		stopHeight = bkstop.Height
		if startHash != empty {
			bkstart, err := chain.DefaultLedger.Store.GetHeader(startHash)
			if err != nil {
				return nil, err
			}
			startHeight = bkstart.Height

			// avoid unsigned integer underflow
			if stopHeight < startHeight {
				return nil, errors.New("do not have header to send")
			}
			count = stopHeight - startHeight

			if count >= MAXINVHDRCNT {
				count = MAXINVHDRCNT
			}
		} else {
			if stopHeight > MAXINVHDRCNT {
				count = MAXINVHDRCNT
			} else {
				count = stopHeight
			}
		}
	}

	tmpBuffer := bytes.NewBuffer([]byte{})
	var i uint32
	for i = 1; i <= count; i++ {
		//FIXME need add error handle for GetBlockWithHash
		hash, _ := chain.DefaultLedger.Store.GetBlockHash(startHeight + i)
		hash.Serialize(tmpBuffer)
	}

	return &InvPayload{
		Type: BLOCK,
		Cnt:  count,
		Blk:  tmpBuffer.Bytes(),
	}, nil
}
