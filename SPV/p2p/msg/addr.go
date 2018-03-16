package msg

import (
	"fmt"
	"time"
	"net"
)

type PeerAddr struct {
	Time     int64
	Services uint64
	IP       [16]byte
	Port     uint16
	ID       uint64 // Unique ID
}

func NewPeerAddr(services uint64, ip [16]byte, port uint16, id uint64) *PeerAddr {
	return &PeerAddr{
		Time:     time.Now().UnixNano(),
		Services: services,
		IP:       ip,
		Port:     port,
		ID:       id,
	}
}

func (addr *PeerAddr) TCPAddr() string {
	var ip net.IP = addr.IP[:]
	return fmt.Sprint(ip.String(), ":", addr.Port)
}
