package test

import "testing"

func SkipShort(t *testing.T) {
	if testing.Short() {
		t.Skip("skipping testing in short mode")
	}
}
