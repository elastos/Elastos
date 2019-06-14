package contract

import (
	"errors"

	"github.com/elastos/Elastos.ELA/crypto"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/common"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/types"
)

//create a Single Singature contract for owner
func CreateSignatureContract(ownerPubKey *crypto.PublicKey) (*types.Contract, error) {
	msg := "[Contract],CreateSignatureContract failed."
	temp, err := ownerPubKey.EncodePoint(true)
	if err != nil {
		return nil, errors.New(msg)
	}
	signatureReedScript, err := CreateSignatureRedeemScript(ownerPubKey)
	if err != nil {
		return nil, errors.New(msg)
	}
	hash, err := common.ToProgramHash(temp)
	if err != nil {
		return nil, errors.New(msg)
	}
	signatureReedScriptToCodeHash, err := common.ToProgramHash(signatureReedScript)
	if err != nil {
		return nil, errors.New(msg)
	}
	return &types.Contract{
		Code: signatureReedScript,
		Parameters: []types.ContractParameterType{types.Signature},
		ProgramHash: *signatureReedScriptToCodeHash,
		OwnerPubkeyHash: *hash,
	}, nil
}

func CreateSignatureRedeemScript(pubkey *crypto.PublicKey) ([]byte, error) {
	temp, err := pubkey.EncodePoint(true)
	if err != nil {
		return nil, errors.New("[Contract],CreateSignatureRedeemScript failed.")
	}
	sb := NewProgramBuilder()
	sb.PushData(temp)
	sb.AddOp(avm.CHECKSIG)
	return sb.ToArray(), nil
}

