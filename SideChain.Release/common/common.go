package common

import (
	"bytes"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
)

func GetGenesisAddress(genesisHash common.Uint256) (string, error) {
	programHash, err := GetGenesisProgramHash(genesisHash)
	if err != nil {
		return "", err
	}
	return programHash.ToAddress()
}

func GetGenesisProgramHash(genesisHash common.Uint256) (*common.Uint168, error) {
	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(genesisHash.Bytes())))
	buf.Write(genesisHash.Bytes())
	buf.WriteByte(byte(common.CROSSCHAIN))

	return crypto.ToProgramHash(buf.Bytes())
}
