// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package outputpayload

import (
	"bytes"
	"github.com/stretchr/testify/assert"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	PUBLICKEY1 = "03e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd"
	PUBLICKEY2 = "023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc77a"
	PUBLICKEY3 = "030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be9"
	PUBLICKEY4 = "0288e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7"
	VOTEHEX    = "000200042103e281f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a3" +
		"8bbeae85dd21023a133480176214f88848c6eaa684a54b316849df2b8570b57f3a917f19bbc" +
		"77a21030a26f8b4ab0ea219eb461d1e454ce5f0bd0d289a6a64ffc0743dab7bd5be0be92102" +
		"88e79636e41edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c701022103e28" +
		"1f89d85b3a7de177c240c4961cb5b1f2106f09daa42d15874a38bbeae85dd210288e79636e4" +
		"1edce04d4fa95d8f62fed73a76164f8631ccc42f5425f960e4a0c7"
)

var (
	candidate1, _ = common.HexStringToBytes(PUBLICKEY1)
	candidate2, _ = common.HexStringToBytes(PUBLICKEY2)
	candidate3, _ = common.HexStringToBytes(PUBLICKEY3)
	candidate4, _ = common.HexStringToBytes(PUBLICKEY4)
)

func TestVoteOutput_Serialize(t *testing.T) {
	voteContent1 := VoteContent{
		VoteType: Delegate,
		CandidateVotes: []CandidateVotes{
			{candidate1, 0},
			{candidate2, 0},
			{candidate3, 0},
			{candidate4, 0},
		},
	}
	voteContent2 := VoteContent{
		VoteType: CRC,
		CandidateVotes: []CandidateVotes{
			{candidate1, 0},
			{candidate4, 0},
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
		t.Error("vote output serialize failed\n", common.BytesToHexString(buf.Bytes()))
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
	if len(vo.Contents[0].CandidateVotes) != 4 ||
		!bytes.Equal(vo.Contents[0].CandidateVotes[0].Candidate, candidate1) ||
		!bytes.Equal(vo.Contents[0].CandidateVotes[1].Candidate, candidate2) ||
		!bytes.Equal(vo.Contents[0].CandidateVotes[2].Candidate, candidate3) ||
		!bytes.Equal(vo.Contents[0].CandidateVotes[3].Candidate, candidate4) {
		t.Error("error candidates in content0")
	}
	if vo.Contents[1].VoteType != CRC {
		t.Error("error vote type in content1")
	}
	if len(vo.Contents[1].CandidateVotes) != 2 ||
		!bytes.Equal(vo.Contents[1].CandidateVotes[0].Candidate, candidate1) ||
		!bytes.Equal(vo.Contents[1].CandidateVotes[1].Candidate, candidate4) {
		t.Error("error candidates in content1")
	}
}

func TestVoteOutput_Validate(t *testing.T) {
	// vo0
	var vo0 *VoteOutput
	err := vo0.Validate()
	assert.EqualError(t, err, "vote output payload is nil")

	// vo1
	content1 := VoteContent{
		VoteType:       Delegate,
		CandidateVotes: []CandidateVotes{},
	}
	vo1 := VoteOutput{
		Version: 0,
		Contents: []VoteContent{
			content1,
		},
	}
	err = vo1.Validate()
	assert.EqualError(t, err, "invalid public key count")

	// vo2
	content2 := VoteContent{
		VoteType: Delegate,
		CandidateVotes: []CandidateVotes{
			{candidate1, 0},
		},
	}
	content3 := VoteContent{
		VoteType: Delegate,
		CandidateVotes: []CandidateVotes{
			{candidate1, 0},
		},
	}
	vo2 := VoteOutput{
		Version: 0,
		Contents: []VoteContent{
			content2,
			content3,
		},
	}
	err = vo2.Validate()
	assert.EqualError(t, err, "duplicate vote type")

	// vo3
	content4 := VoteContent{
		VoteType: Delegate,
		CandidateVotes: []CandidateVotes{
			{candidate1, 0},
			{candidate1, 0},
		},
	}
	vo3 := VoteOutput{
		Version: 0,
		Contents: []VoteContent{
			content4,
		},
	}
	err = vo3.Validate()
	assert.EqualError(t, err, "duplicate candidate")

	// vo4
	resBytes, _ := common.HexStringToBytes(VOTEHEX)
	buf := bytes.NewBuffer(resBytes)
	var vo4 VoteOutput
	if err := vo4.Deserialize(buf); err != nil {
		t.Error("vote output deserialize failed")
	}
	err = vo4.Validate()
	assert.NoError(t, err)
}
