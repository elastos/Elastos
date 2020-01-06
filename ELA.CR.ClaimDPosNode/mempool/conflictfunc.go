package mempool

import (
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/errors"
)

// hashes related functions
var (

	hashGetTx = func(tx *types.Transaction) (interface{}, error) {
		return tx.Hash(), nil
	}
)

// strings related functions
var (

	strGetDPoSCancelProducer = func(tx *types.Transaction) (interface{},
		error) {
		p, ok := tx.Payload.(*payload.ProcessProducer)
		if !ok {
			err := fmt.Errorf(
				"cancel producer payload cast failed, tx:%s", tx.Hash())
			return nil, errors.Simple(errors.ErrTxPoolFailure, err)
		}
		return common.BytesToHexString(p.OwnerPublicKey), nil
	}
)

// program hashes related functions
var (

	addrGetCRMemberDID = func(tx *types.Transaction) (interface{}, error) {
		p, ok := tx.Payload.(*payload.CRInfo)
		if !ok {
			return nil, fmt.Errorf(
				"CRInfo payload cast failed, tx:%s", tx.Hash())
		}
		return p.DID, nil
	}
)
