package types

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
)

const (
	VOTEPAYLOADHEX = "0002000000000400000021036e86f83253e88acfc9cd771e344d178e9c31ed21f409e6f52d4766d0151e4c644af3bc0884bf916521aecfab7a57ed1d05cfda1d930bde6669b90db32321e9084586d4a5c48651db79e8d6718fc4fb7c3b27010200000021036e86f83253e88acfc9cd771e344d178e9c31ed21e9084586d4a5c48651db79e8d6718fc4fb7c3b27"
	OUTPUTHEX      = "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0a08601000000000000000000210fcd528848be05f8cffe5d99896c44bdeec70502010002000000000400000021036e86f83253e88acfc9cd771e344d178e9c31ed21f409e6f52d4766d0151e4c644af3bc0884bf916521aecfab7a57ed1d05cfda1d930bde6669b90db32321e9084586d4a5c48651db79e8d6718fc4fb7c3b27010200000021036e86f83253e88acfc9cd771e344d178e9c31ed21e9084586d4a5c48651db79e8d6718fc4fb7c3b27"
)

var (
	assetID, _   = common.Uint256FromHexString("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0")
	recipient, _ = common.Uint168FromAddress("EJbTbWd8a9rdutUfvBxhcrvEeNy21tW1Ee")
)

func TestOutput_Serialize(t *testing.T) {
	// C0
	voteBytes, _ := common.HexStringToBytes(VOTEPAYLOADHEX)
	voteBuf := bytes.NewBuffer(voteBytes)

	var vo outputpayload.VoteOutput
	if err := vo.Deserialize(voteBuf); err != nil {
		t.Error("vote output deserialize failed")
	}

	output := Output{
		AssetID:       *assetID,
		Value:         100000,
		OutputLock:    0,
		ProgramHash:   *recipient,
		OutputType:    VoteOutput,
		OutputPayload: &vo,
	}

	buf := new(bytes.Buffer)
	if err := output.Serialize(buf, TxVersion09); err != nil {
		t.Error("output serialize failed")
	}

	resBytes, _ := common.HexStringToBytes(OUTPUTHEX)
	if !bytes.Equal(buf.Bytes(), resBytes) {
		t.Error("output serialize failed")
	}
}

func TestOutput_Deserialize(t *testing.T) {
	// C0
	outputBytes, _ := common.HexStringToBytes(OUTPUTHEX)
	outputBuf := bytes.NewBuffer(outputBytes)

	var output Output
	if err := output.Deserialize(outputBuf, TxVersion09); err != nil {
		t.Error("output deserialize failed")
	}

	buf := new(bytes.Buffer)
	if err := output.Serialize(buf, TxVersion09); err != nil {
		t.Error("output serialize failed")
	}

	resBytes, _ := common.HexStringToBytes(OUTPUTHEX)
	if !bytes.Equal(buf.Bytes(), resBytes) {
		t.Error("output deserialize failed")
	}
}
