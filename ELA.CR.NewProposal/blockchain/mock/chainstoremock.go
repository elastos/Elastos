package mock

import (
	"github.com/elastos/Elastos.ELA/core/types/payload"
)

type ChainStoreMock struct {
	RegisterProducers []*payload.PayloadRegisterProducer
	BlockHeight uint32
}
