package core

import (
	"math"
	"testing"
)

func TestHeightVersions_GetCheckFlags(t *testing.T) {

	if BlockHeightVersions.GetCheckFlags(0) != 0 ||
		BlockHeightVersions.GetCheckFlags(88811) != 0 {
		t.Error("Version 0 flags error")
	}

	if BlockHeightVersions.GetCheckFlags(88812)&CheckTxOut == 0 ||
		BlockHeightVersions.GetCheckFlags(108811)&CheckTxOut == 0 {
		t.Error("Version 1 flags error")
	}

	if BlockHeightVersions.GetCheckFlags(108812)&CheckTxOut == 0 ||
		BlockHeightVersions.GetCheckFlags(108812)&CheckCoinbaseTxDposReward == 0 ||
		BlockHeightVersions.GetCheckFlags(math.MaxUint32)&CheckTxOut == 0 ||
		BlockHeightVersions.GetCheckFlags(math.MaxUint32)&CheckCoinbaseTxDposReward == 0 {
		t.Error("Version 2 flags error")
	}
}
