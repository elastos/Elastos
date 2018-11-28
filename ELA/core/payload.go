package core

import (
	"errors"
	"io"
)

const (
	// MaxPayloadDataSize is the maximum allowed length of payload data.
	MaxPayloadDataSize = 1024 * 1024 // 1MB
)

// Payload define the func for loading the payload data
// base on payload type which have different structure
type Payload interface {
	// Get payload data
	Data(version byte) []byte

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
	case RegisterProducer:
		p = new(PayloadRegisterProducer)
	case CancelProducer:
		p = new(PayloadCancelProducer)
	case UpdateProducer:
		p = &PayloadUpdateProducer{
			new(PayloadRegisterProducer),
		}
		p = new(PayloadUpdateProducer)
	case IllegalProposalEvidence:
		p = new(PayloadIllegalProposal)
	case IllegalVoteEvidence:
		p = new(PayloadIllegalVote)
	default:
		return nil, errors.New("[Transaction], invalid transaction type.")
	}
	return p, nil
}
