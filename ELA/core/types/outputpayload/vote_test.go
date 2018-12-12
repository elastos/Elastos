package outputpayload

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	ADDRESS1 = "EHU3vj4AbaVSB78otYm57kWYXoBL4JKqg3"
	ADDRESS2 = "EfQGEQWtur9Ydvho61deHm3XfgZNPJ4mkW"
	ADDRESS3 = "EZ6Dr7fuRvwAwNSvtysc8sWf66MuTYCDPt"
	ADDRESS4 = "EeQ4qzzkGNmJpScU1UNgrGAQCi61AvwUxj"
	VOTEHEX  = "0002000000000400000021036e86f83253e88acfc9cd771e344d178e9c31ed21f409e6f52d4766d0151e4c644af3bc0884bf916521aecfab7a57ed1d05cfda1d930bde6669b90db32321e9084586d4a5c48651db79e8d6718fc4fb7c3b27010200000021036e86f83253e88acfc9cd771e344d178e9c31ed21e9084586d4a5c48651db79e8d6718fc4fb7c3b27"
)

var (
	candidate1, _ = common.Uint168FromAddress(ADDRESS1)
	candidate2, _ = common.Uint168FromAddress(ADDRESS2)
	candidate3, _ = common.Uint168FromAddress(ADDRESS3)
	candidate4, _ = common.Uint168FromAddress(ADDRESS4)
)

func TestVoteOutput_Serialize(t *testing.T) {
	voteContent1 := VoteContent{
		VoteType: Delegate,
		Candidates: []common.Uint168{
			*candidate1,
			*candidate2,
			*candidate3,
			*candidate4,
		},
	}
	voteContent2 := VoteContent{
		VoteType: CRC,
		Candidates: []common.Uint168{
			*candidate1,
			*candidate4,
		},
	}
	voteOutput := VoteOutput{
		Version: 0,
		Contents: []VoteContent{
			voteContent1,
			voteContent2,
		},
	}

	buf := new(bytes.Buffer)
	if err := voteOutput.Serialize(buf); err != nil {
		t.Error("vote output serialize failed")
	}

	resBytes, _ := common.HexStringToBytes(VOTEHEX)
	if !bytes.Equal(buf.Bytes(), resBytes) {
		t.Error("vote output serialize failed")
	}
}

func TestVoteOutput_Deserialize(t *testing.T) {
	resBytes, _ := common.HexStringToBytes(VOTEHEX)
	buf := bytes.NewBuffer(resBytes)

	var vo VoteOutput
	if err := vo.Deserialize(buf); err != nil {
		t.Error("vote output deserialize failed")
	}

	if vo.Version != 0 {
		t.Error("error version")
	}
	if len(vo.Contents) != 2 {
		t.Error("error contents count")
	}
	if vo.Contents[0].VoteType != Delegate {
		t.Error("error vote type in content0")
	}
	if len(vo.Contents[0].Candidates) != 4 ||
		!vo.Contents[0].Candidates[0].IsEqual(*candidate1) ||
		!vo.Contents[0].Candidates[1].IsEqual(*candidate2) ||
		!vo.Contents[0].Candidates[2].IsEqual(*candidate3) ||
		!vo.Contents[0].Candidates[3].IsEqual(*candidate4) {
		t.Error("error candidates in content0")
	}
	if vo.Contents[1].VoteType != CRC {
		t.Error("error vote type in content1")
	}
	if len(vo.Contents[1].Candidates) != 2 ||
		!vo.Contents[1].Candidates[0].IsEqual(*candidate1) ||
		!vo.Contents[1].Candidates[1].IsEqual(*candidate4) {
		t.Error("error candidates in content1")
	}
}
