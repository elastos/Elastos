package account

import (
	"encoding/hex"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	MAINACCOUNT      = "main-account"
	SUBACCOUNT       = "sub-account"
	KeystoreFileName = "keystore.dat"
	KeystoreVersion  = "1.0.0"

	MaxSignalQueueLen = 5
)

var IDReverse, _ = hex.DecodeString("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0")
var SystemAssetID, _ = common.Uint256FromBytes(common.BytesReverse(IDReverse))
