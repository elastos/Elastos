package account

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/account"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
	"github.com/elastos/Elastos.ELA/crypto"
)

type Account interface {
	PublicKey() *crypto.PublicKey
	PublicKeyBytes() []byte
	SignProposal(proposal *payload.DPOSProposal) ([]byte, error)
	SignVote(vote *payload.DPOSProposalVote) ([]byte, error)
	Sign(data []byte) []byte
	SignTx(tx *types.Transaction) ([]byte, error)
	DecryptAddr(cipher []byte) (addr string, err error)
}

type dAccount struct {
	*account.Account
	pubKey []byte
}

func (a *dAccount) PublicKey() *crypto.PublicKey {
	return a.Account.PublicKey
}

func (a *dAccount) PublicKeyBytes() []byte {
	return a.pubKey
}

func (a *dAccount) SignProposal(proposal *payload.DPOSProposal) ([]byte,
	error) {
	privateKey := a.PrivKey()

	signature, err := crypto.Sign(privateKey, proposal.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
}

func (a *dAccount) SignVote(vote *payload.DPOSProposalVote) ([]byte, error) {
	privateKey := a.PrivKey()

	signature, err := crypto.Sign(privateKey, vote.Data())
	if err != nil {
		return []byte{0}, err
	}

	return signature, nil
}

func (a *dAccount) Sign(data []byte) []byte {
	privateKey := a.PrivKey()
	sign, err := crypto.Sign(privateKey, data)
	if err != nil {
		return nil
	}
	return sign
}

func (a *dAccount) SignTx(tx *types.Transaction) ([]byte, error) {
	buf := new(bytes.Buffer)
	if err := tx.SerializeUnsigned(buf); err != nil {
		return nil, err
	}

	privateKey := a.PrivKey()
	return crypto.Sign(privateKey, buf.Bytes())
}

func (a *dAccount) DecryptAddr(cipher []byte) (addr string, err error) {
	data, err := crypto.Decrypt(a.PrivateKey, cipher)
	return string(data), err
}

func Open(password []byte) (Account, error) {
	client, err := account.Open(account.KeystoreFileName, password)
	if err != nil {
		return nil, err
	}
	a := client.GetMainAccount()
	pk, err := a.PublicKey.EncodePoint(true)
	if err != nil {
		return nil, err
	}

	return &dAccount{Account: a, pubKey: pk}, nil
}

func New(a *account.Account) Account {
	pubKey, _ := a.PublicKey.EncodePoint(true)
	return &dAccount{Account: a, pubKey: pubKey}
}
