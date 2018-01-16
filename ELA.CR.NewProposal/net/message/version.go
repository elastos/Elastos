package message

import (
	"Elastos.ELA/common/config"
	"Elastos.ELA/common/log"
	"Elastos.ELA/core/ledger"
	. "Elastos.ELA/net/protocol"
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"time"
)

type version struct {
	messageHeader
	Body struct {
		Version      uint32
		Services     uint64
		TimeStamp    uint32
		Port         uint16
		Nonce        uint64
		// TODO remove tempory to get serilization function passed
		StartHeight uint64
		// FIXME check with the specify relay type length
		Relay uint8
	}
}

func NewVersion(n Noder) ([]byte, error) {
	log.Debug()
	var msg version

	msg.Body.Version = n.Version()
	msg.Body.Services = n.Services()

	// FIXME Time overflow
	msg.Body.TimeStamp = uint32(time.Now().UTC().UnixNano())
	msg.Body.Port = n.GetPort()
	msg.Body.Nonce = n.GetID()
	msg.Body.StartHeight = uint64(ledger.DefaultLedger.GetLocalBlockChainHeight())
	if n.GetRelay() {
		msg.Body.Relay = 1
	} else {
		msg.Body.Relay = 0
	}

	msg.Magic = config.Parameters.Magic
	copy(msg.CMD[0:7], "version")
	p := bytes.NewBuffer([]byte{})
	err := binary.Write(p, binary.LittleEndian, &(msg.Body))
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

	m, err := msg.Serialization()
	if err != nil {
		log.Error("Error Convert net message ", err.Error())
		return nil, err
	}

	return m, nil
}

func (msg version) Serialization() ([]byte, error) {
	hdrBuf, err := msg.messageHeader.Serialization()
	if err != nil {
		return nil, err
	}
	buf := bytes.NewBuffer(hdrBuf)
	err = binary.Write(buf, binary.LittleEndian, msg.Body)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), err
}

func (msg *version) Deserialization(p []byte) error {
	buf := bytes.NewBuffer(p)

	err := binary.Read(buf, binary.LittleEndian, &(msg.messageHeader))
	if err != nil {
		log.Warn("Parse version message hdr error")
		return errors.New("Parse version message hdr error")
	}

	err = binary.Read(buf, binary.LittleEndian, &(msg.Body))
	if err != nil {
		log.Warn("Parse version Body message error")
		return errors.New("Parse version Body message error")
	}

	return err
}

/*
 * The node state switch table after rx message, there is time limitation for each action
 * The Handshake status will switch to Init after TIMEOUT if not received the VerACK
 * in this time window
 *  _______________________________________________________________________
 * |          |    Init         | HandShake |  Establish | Inactive      |
 * |-----------------------------------------------------------------------|
 * | version  | HandShake(timer)|           |            | HandShake(timer)|
 * |          | if helloTime > 3| Tx verack | Depend on  | if helloTime > 3|
 * |          | Tx version      |           | node update| Tx version      |
 * |          | then Tx verack  |           |            | then Tx verack  |
 * |-----------------------------------------------------------------------|
 * | verack   |                 | Establish |            |                 |
 * |          |   No Action     |           | No Action  | No Action       |
 * |------------------------------------------------------------------------
 */
func (msg version) Handle(node Noder) error {
	log.Debug()
	localNode := node.LocalNode()

	// Exclude the node itself
	if msg.Body.Nonce == localNode.GetID() {
		log.Warn("The node handshake with itself")
		node.CloseConn()
		return errors.New("The node handshake with itself")
	}

	s := node.GetState()
	if s != Init && s != Hand {
		log.Warn("Unknow status to receive version")
		return errors.New("Unknow status to receive version")
	}

	// Obsolete node
	n, ret := localNode.DelNbrNode(msg.Body.Nonce)
	if ret == true {
		log.Info(fmt.Sprintf("Node reconnect 0x%x", msg.Body.Nonce))
		// Close the connection and release the node soure
		n.SetState(Inactive)
		n.CloseConn()
	}

	node.UpdateInfo(time.Now(), msg.Body.Version, msg.Body.Services,
		msg.Body.Port, msg.Body.Nonce, msg.Body.Relay, msg.Body.StartHeight)
	localNode.AddNbrNode(node)

	ip, _ := node.GetAddr16()
	addr := NodeAddr{
		Time:     node.GetTime(),
		Services: msg.Body.Services,
		IpAddr:   ip,
		Port:     msg.Body.Port,
		ID:       msg.Body.Nonce,
	}
	localNode.AddAddressToKnownAddress(addr)

	var buf []byte
	if s == Init {
		node.SetState(HandShake)
		buf, _ = NewVersion(localNode)
	} else if s == Hand {
		node.SetState(HandShaked)
		buf, _ = NewVerack()
	}
	node.Tx(buf)

	return nil
}
