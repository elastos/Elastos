package p2p

import (
	"fmt"
	"net"
	"time"
)

type NetAddress struct {
	Time     int64
	Services uint64
	IP       [16]byte
	Port     uint16
	ID       uint64 // Unique ID
}

func NewNetAddress(services uint64, ip [16]byte, port uint16, id uint64) *NetAddress {
	return &NetAddress{
		Time:     time.Now().UnixNano(),
		Services: services,
		IP:       ip,
		Port:     port,
		ID:       id,
	}
}

func (addr NetAddress) String() string {
	var ip net.IP = addr.IP[:]
	return fmt.Sprint(ip.String(), ":", addr.Port)
}
