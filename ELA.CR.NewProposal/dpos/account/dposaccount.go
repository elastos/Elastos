package account

import (
	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

type DposAccount interface {
	SignProposal(proposal *core.DPosProposal) ([]byte, error)
	SignVote(vote *core.DPosProposalVote) ([]byte, error)
}

type dposAccount struct {
	keystore interface{} //todo key store place holder
}

func (a *dposAccount) derivePrivateKey() ([]byte, error) {
	//fixme change getting private key from config to key store in future
	return common.HexStringToBytes(config.Parameters.ArbiterConfiguration.PrivateKey)
}

func (a *dposAccount) SignProposal(proposal *core.DPosProposal) ([]byte, error) {
	privateKey, err := a.derivePrivateKey()
	if err != nil {
		return []byte{0}, err
	}

	signature, err := crypto.Sign(privateKey, proposal.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
}

func (a *dposAccount) SignVote(vote *core.DPosProposalVote) ([]byte, error) {
	privateKey, err := a.derivePrivateKey()
	if err != nil {
		return []byte{0}, err
	}

	signature, err := crypto.Sign(privateKey, vote.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
}

func NewDposAccount() DposAccount {
	return &dposAccount{}
}
