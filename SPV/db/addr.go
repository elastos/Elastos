package db

import . "github.com/elastos/Elastos.ELA.SPV/core"

const (
	TypeMaster = 0
	TypeSub    = 1 << 1
	TypeMulti  = 1 << 2
	TypeNotify = 1 << 3
)

type Addr struct {
	hash     *Uint168
	script   []byte
	addrType int
}

func NewAddr(hash *Uint168, script []byte, addrType int) *Addr {
	return &Addr{hash: hash, script: script, addrType: addrType}
}

func (addr *Addr) Hash() *Uint168 {
	return addr.hash
}

func (addr *Addr) String() string {
	str, _ := addr.hash.ToAddress()
	return str
}

func (addr *Addr) Script() []byte {
	return addr.script
}

func (addr *Addr) Type() int {
	return addr.addrType
}

func (addr *Addr) TypeName() string {
	switch addr.addrType {
	case TypeMaster:
		return "MASTER"
	case TypeSub:
		return "SUB"
	case TypeMulti:
		return "MULTI"
	case TypeNotify:
		return "NOTIFY"
	default:
		return ""
	}
}
