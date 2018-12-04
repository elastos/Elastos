package core

import (
	"errors"
	"io"
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

func GetPayload(txType TransactionType) (Payload, error) {
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
	case SideChainPow:
		p = new(PayloadSideChainPow)
	case WithdrawFromSideChain:
		p = new(PayloadWithdrawFromSideChain)
	case TransferCrossChainAsset:
		p = new(PayloadTransferCrossChainAsset)
	default:
		return nil, errors.New("[Transaction], invalid transaction type.")
	}
	return p, nil
}
