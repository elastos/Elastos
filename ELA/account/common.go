package account

import (
	"encoding/hex"
	"errors"

	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	MAINACCOUNT      = "main-account"
	SUBACCOUNT       = "sub-account"
	KeystoreFileName = "keystore.dat"
	KeystoreVersion  = "1.0.0"

	MaxSignalQueueLen = 5
)

var IDReverse, _ = hex.DecodeString("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0")
var SystemAssetID, _ = Uint256FromBytes(BytesReverse(IDReverse))

func GetSigner(code []byte) (*Uint168, error) {
	if len(code) != crypto.PublicKeyScriptLength || code[len(code)-1] != STANDARD {
		return nil, errors.New("not a valid standard transaction code, length not match")
	}

	contract, err := contract.CreateStandardContractByCode(code)
	if err != nil {
		return nil, err
	}

	return contract.ToProgramHash()
}

func GetSigners(code []byte) ([]*Uint168, error) {
	publicKeys, err := crypto.ParseMultisigScript(code)
	if err != nil {
		return nil, err
	}

	var signers []*Uint168
	for _, publicKey := range publicKeys {
		hash, err := contract.PublicKeyToStandardProgramHash(publicKey)
		if err != nil {
			return nil, err
		}
		signers = append(signers, hash)
	}

	return signers, nil
}
