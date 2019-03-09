package contract

import (
	"bytes"

	"github.com/elastos/Elastos.ELA/common"
)

// A temporary file created for cross chain tx
//
//func CreateCrossChainContract(m int, pubkeys []*crypto.PublicKey) (*Contract, error) {
//	if !(m >= 1 && m <= len(pubkeys) && len(pubkeys) <= 24) {
//		return nil, nil //TODO: add panic
//	}
//
//	sb := pg.NewProgramBuilder()
//	sb.PushNumber(big.NewInt(int64(m)))
//
//	//sort pubkey
//	crypto.SortPublicKeys(pubkeys)
//
//	for _, pubkey := range pubkeys {
//		temp, err := pubkey.EncodePoint(true)
//		if err != nil {
//			return nil, errors.New("[Contract],CreateCrossChainContract failed")
//		}
//		sb.PushData(temp)
//	}
//
//	sb.PushNumber(big.NewInt(int64(len(pubkeys))))
//	sb.AddOp(vm.CHECKMULTISIG)
//
//	return &Contract{
//		Code:   sb.ToArray(),
//		Prefix: PrefixMultiSig,
//	}, nil
//}

func CreateCrossChainRedeemScript(genesisHash common.Uint256) []byte {
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(genesisHash.Bytes())))
	buf.Write(genesisHash.Bytes())
	buf.WriteByte(byte(CROSSCHAIN))

	return buf.Bytes()
}
