package message

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"fmt"
	"net"
	"strconv"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/net/protocol"
)

type addr struct {
	Hdr
	nodeCnt   uint64
	nodeAddrs []NodeAddr
}

func NewAddrs(nodeaddrs []NodeAddr, count uint64) ([]byte, error) {
	var msg addr
	msg.nodeAddrs = nodeaddrs
	msg.nodeCnt = count
	msg.Magic = config.Parameters.Magic
	cmd := "addr"
	copy(msg.CMD[0:7], cmd)
	p := new(bytes.Buffer)
	err := binary.Write(p, binary.LittleEndian, msg.nodeCnt)
	if err != nil {
		log.Error("Binary Write failed at new Msg: ", err.Error())
		return nil, err
	}

	err = binary.Write(p, binary.LittleEndian, msg.nodeAddrs)
	if err != nil {
		log.Error("Binary Write failed at new Msg: ", err.Error())
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

func (msg addr) Serialize() ([]byte, error) {
	var buf bytes.Buffer
	err := binary.Write(&buf, binary.LittleEndian, msg.Hdr)

	if err != nil {
		return nil, err
	}
	err = binary.Write(&buf, binary.LittleEndian, msg.nodeCnt)
	if err != nil {
		return nil, err
	}
	for _, v := range msg.nodeAddrs {
		err = binary.Write(&buf, binary.LittleEndian, v)
		if err != nil {
			return nil, err
		}
	}

	return buf.Bytes(), err
}

func (msg *addr) Deserialize(p []byte) error {
	buf := bytes.NewBuffer(p)
	err := binary.Read(buf, binary.LittleEndian, &(msg.Hdr))
	err = binary.Read(buf, binary.LittleEndian, &(msg.nodeCnt))
	log.Debug("The address count is ", msg.nodeCnt)
	msg.nodeAddrs = make([]NodeAddr, msg.nodeCnt)
	for i := 0; i < int(msg.nodeCnt); i++ {
		err := binary.Read(buf, binary.LittleEndian, &(msg.nodeAddrs[i]))
		if err != nil {
			goto err
		}
	}
err:
	return err
}

func (msg addr) Handle(node Noder) error {
	log.Debug()
	for _, v := range msg.nodeAddrs {
		var ip net.IP
		ip = v.IpAddr[:]
		//address := ip.To4().String() + ":" + strconv.Itoa(int(v.Port))
		address := ip.To16().String() + ":" + strconv.Itoa(int(v.Port))
		log.Info(fmt.Sprintf("The ip address is %s id is 0x%x", address, v.ID))

		if v.ID == node.LocalNode().ID() {
			continue
		}
		if node.LocalNode().NodeEstablished(v.ID) {
			continue
		}

		if v.Port == 0 {
			continue
		}

		//save the node address in address list
		node.LocalNode().AddAddressToKnownAddress(v)
	}
	return nil
}
