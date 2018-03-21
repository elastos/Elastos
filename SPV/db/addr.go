package db

import . "SPVWallet/core"

type Addr struct {
	hash   *Uint168
	script []byte
}

func NewAddr(hash *Uint168, script []byte) *Addr {
	return &Addr{hash: hash, script: script}
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
