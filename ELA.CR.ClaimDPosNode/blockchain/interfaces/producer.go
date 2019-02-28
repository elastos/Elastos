package interfaces

import (
	"github.com/elastos/Elastos.ELA/common"
)

type Producer interface {
	Votes() common.Fixed64
	NodePublicKey() []byte
	OwnerPublicKey() []byte
}
