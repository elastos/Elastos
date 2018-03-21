package _interface

import (
	"SPVWallet/p2p/msg"
	tx "SPVWallet/core/transaction"
)

type SPVService interface {
	RegisterAccount(address string) error
	OnTransactionConfirmed(func(msg.MerkleBlock, []tx.Transaction))
	VerifyTransaction(msg.MerkleBlock, []tx.Transaction) error
	SendTransaction(tx.Transaction) error
	Start() error
}

func NewSPVService(clientId uint64) (SPVService, error) {
	return &SPVServiceImpl{
		clientId: clientId,
	}, nil
}

type SPVServiceImpl struct {
	clientId uint64
}

func (service *SPVServiceImpl) RegisterAccount(address string) error {
	return nil
}

func (service *SPVServiceImpl) OnTransactionConfirmed(callback func(msg.MerkleBlock, []tx.Transaction)) {

}

func (service *SPVServiceImpl) VerifyTransaction(block msg.MerkleBlock, txs []tx.Transaction) error {
	return nil
}

func (service *SPVServiceImpl) SendTransaction(tx tx.Transaction) error {
	return nil
}

func (service *SPVServiceImpl) Start() error {
	return nil
}
