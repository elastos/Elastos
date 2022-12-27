package types

import (
	"errors"
	"io"
)

const (
	// MaxPayloadDataSize is the maximum allowed length of payload data.
	MaxPayloadDataSize = 1024 * 1024 // 1MB
)

//Payload define the func for loading the payload data
//base on payload type which have different struture
type Payload interface {
	//  Get payload data
	Data(version byte) []byte

	//Serialize payload data
	Serialize(w io.Writer, version byte) error

	Deserialize(r io.Reader, version byte) error
}

var GetPayloadByTxType = func(txType TxType) (Payload, error) {
	var p Payload
	switch txType {
	case CoinBase:
		p = new(PayloadCoinBase)
	case RegisterAsset:
		p = new(PayloadRegisterAsset)
	case TransferAsset:
		p = new(PayloadTransferAsset)
	case Record:
		p = new(PayloadRecord)
	case RechargeToSideChain:
		p = new(PayloadRechargeToSideChain)
	case TransferCrossChainAsset:
		p = new(PayloadTransferCrossChainAsset)
	default:
		return nil, errors.New("[Transaction], invalid transaction type.")
	}
	return p, nil
}