package config

import (
	"testing"

	"github.com/stretchr/testify/assert"
)

func TestGenesisBlock(t *testing.T) {
	block := GenesisBlock(&mainNetFoundation)
	assert.Equal(t, len(block.Transactions), 2)

	genesisHash := block.Hash().String()
	assert.Equal(t, "8d7014f2f941caa1972c8033b2f0a860ec8d4938b12bae2c62512852a558f405", genesisHash)

	genesisHash = GenesisBlock(&testNetFoundation).Hash().String()
	assert.Equal(t, "b3314f465ea5556d570bcc473d59a0855b4405a25b1ea0c957c81b2920be1864", genesisHash)
}
