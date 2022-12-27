package test

import (
	"path/filepath"
	"testing"
)

var (
	// DataDir indicates the root directory of the test
	DataDir = "elastos_test"

	// DataPath indicates the path storing the chain data in test
	DataPath = filepath.Join(DataDir, "data")

	// NodeLogPath indicates the path storing the node log in test
	NodeLogPath = filepath.Join(DataDir, "logs/node")

	// DPoSLogPath indicates the path storing the DPoS log in test
	DPoSLogPath = filepath.Join(DataDir, "logs/dpos")
)

// SkipShort is used to skip the following testing in short mode
func SkipShort(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping testing in short mode")
	}
}
