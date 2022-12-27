package smartcontract

import (
	"io"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/interfaces"
)

//SignableData describe the data need be signed.
type SignableData interface {
	interfaces.IDataContainer
	//TODO: add SerializeUnsigned
	SerializeUnsigned(io.Writer) error
}
