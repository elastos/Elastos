package account

import (
	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/common/password"
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA/crypto"
)

type DposAccount interface {
	SignProposal(proposal *core.DPosProposal) ([]byte, error)
	SignVote(vote *core.DPosProposalVote) ([]byte, error)
	SignPeerNonce(nonce []byte) (signature [64]byte)
}

type dposAccount struct {
	account.Account
}

func (a *dposAccount) SignProposal(proposal *core.DPosProposal) ([]byte, error) {
	privateKey := a.PrivKey()

	signature, err := crypto.Sign(privateKey, proposal.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
}

func (a *dposAccount) SignVote(vote *core.DPosProposalVote) ([]byte, error) {
	privateKey := a.PrivKey()

	signature, err := crypto.Sign(privateKey, vote.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
}

func (a *dposAccount) SignPeerNonce(nonce []byte) (signature [64]byte) {
	privateKey := a.PrivKey()

	sign, err := crypto.Sign(privateKey, nonce)
	if err != nil || len(signature) != 64 {
		return signature
	}

	copy(signature[:], sign)

	return signature
}

func NewDposAccount() (DposAccount, error) {
	password, err := password.GetPassword()
	if err != nil {
		return nil, err
	}
	client, err := account.Open(account.KeystoreFileName, password)
	if err != nil {
		return nil, err
	}
	acc, err := client.GetDefaultAccount()
	if err != nil {
		return nil, err
	}

	return &dposAccount{*acc}, nil
}
