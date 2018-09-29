package core

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
var PayloadHelper *PayloadBase

type Payload interface {
	//  Get payload data
	Data(version byte) []byte

	//Serialize payload data
	Serialize(w io.Writer, version byte) error

	Deserialize(r io.Reader, version byte) error
}

type PayloadBase struct {
	GetPayload func(txType TransactionType) (Payload, error)
}

func InitPayloadCreater() {
	PayloadHelper = &PayloadBase{}
	PayloadHelper.Init()
}

func (pb *PayloadBase) Init() {
	pb.GetPayload = pb.getPayload
}

func (pb *PayloadBase) getPayload(txType TransactionType) (Payload, error) {
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
