package store

import (
	"crypto/rand"
	"github.com/elastos/Elastos.ELA/common"
	"os"
	"testing"

	"github.com/stretchr/testify/assert"
)

func mockPubKeys(count int) [][]byte {
	pubKeys := make([][]byte, 0, count)
	for i := 0; i < count; i++ {
		pubKey := make([]byte, 32)
		rand.Read(pubKey)
		pubKeys = append(pubKeys, pubKey)
	}
	return pubKeys
}

func mockBlockHashes(count int) []*common.Uint256 {
	hashes := make([]*common.Uint256, 0, count)
	for i := 0; i < count; i++ {
		var hash common.Uint256
		rand.Read(hash[:])
		hashes = append(hashes, &hash)
	}
	return hashes
}

func TestDposStore(t *testing.T) {
	store, err := NewDPOSStore("test")
	assert.NoError(t, err)
	defer os.RemoveAll("test")

	count := 10
	ownerPublicKeys := mockPubKeys(count)
	sideProducerIDs := mockPubKeys(count)
	blockHashes := mockBlockHashes(count)

	for i, ownerPublicKey := range ownerPublicKeys {
		err := store.Mapping(ownerPublicKey, sideProducerIDs[i], uint32(i))
		assert.NoError(t, err)
		err = store.Archive(ownerPublicKeys, uint32(i), blockHashes[i])
		assert.NoError(t, err)
		if i >= 6 {
			producers, err := store.GetProducers(uint32(i))
			assert.NoError(t, err)
			assert.Equal(t, i-5, len(producers))
			assert.Equal(t, sideProducerIDs[:i-5], producers)
		}
	}

	for i := 9; i > 5; i-- {
		assert.NoError(t, store.Rollback())
		producers, err := store.GetProducers(uint32(i))
		assert.Error(t, err)
		producers, err = store.GetProducers(uint32(i - 1))
		assert.NoError(t, err)
		assert.Equal(t, i-6, len(producers))
	}

	for i, hash := range blockHashes {
		blockHash, err := store.GetBlockHash(uint32(i))
		assert.NoError(t, err)
		assert.Equal(t, hash, blockHash)
	}
}
