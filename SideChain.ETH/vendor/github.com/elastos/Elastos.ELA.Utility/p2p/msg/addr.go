package msg

import (
	"fmt"
	"io"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const MaxAddrPerMsg = 1000

// Ensure Addr implement p2p.Message interface.
var _ p2p.Message = (*Addr)(nil)

type Addr struct {
	AddrList []*p2p.NetAddress
}

func NewAddr(addresses []*p2p.NetAddress) *Addr {
	msg := new(Addr)
	msg.AddrList = addresses
	return msg
}

func (msg *Addr) CMD() string {
	return p2p.CmdAddr
}

func (msg *Addr) MaxLength() uint32 {
	return 8 + (MaxAddrPerMsg * 42)
}

func (msg *Addr) Serialize(w io.Writer) error {
	// Protocol versions before MultipleAddressVersion only allowed 1 address
	// per message.
	count := len(msg.AddrList)
	if count > MaxAddrPerMsg {
		str := fmt.Sprintf("too many addresses for message "+
			"[count %v, max %v]", count, MaxAddrPerMsg)
		return common.FuncError("Addr.Serialize", str)
	}

	err := common.WriteUint64(w, uint64(count))
	if err != nil {
		return err
	}

	for _, na := range msg.AddrList {
		if err := na.Serialize(w); err != nil {
			return err
		}
	}
	return nil
}

func (msg *Addr) Deserialize(r io.Reader) error {
	count, err := common.ReadUint64(r)
	if err != nil {
		return err
	}

	// Limit to max addresses per message.
	if count > MaxAddrPerMsg {
		return fmt.Errorf("Addr.Deserialize too many addresses"+
			" for message [count %v, max %v]", count, MaxAddrPerMsg)
	}

	addrList := make([]p2p.NetAddress, count)
	msg.AddrList = make([]*p2p.NetAddress, 0, count)
	for i := uint64(0); i < count; i++ {
		na := &addrList[i]
		if err := na.Deserialize(r); err != nil {
			return err
		}
		msg.AddrList = append(msg.AddrList, na)
	}

	return nil
}
