package db

import (
	"bytes"
	"encoding/hex"
	"errors"
	"fmt"
	"sync"

	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA/bloom"
	"encoding/binary"
)

type Proofs interface {
	// Put a merkle proof of the block
	Put(proof *bloom.MerkleProof) error

	// Get a merkle proof of a block
	Get(height uint32) (*bloom.MerkleProof, error)

	// Get all merkle proofs in database
	GetAll() ([]*bloom.MerkleProof, error)

	// Delete a merkle proof of a block
	Delete(height uint32) error
}

type ProofStore struct {
	*sync.RWMutex
	*bolt.DB
}

var (
	BKTProofs = []byte("Proofs")
)

func NewProofsDB(db *bolt.DB) (*ProofStore, error) {
	db.Update(func(btx *bolt.Tx) error {
		_, err := btx.CreateBucketIfNotExists(BKTProofs)
		if err != nil {
			return err
		}
		return nil
	})

	return &ProofStore{RWMutex: new(sync.RWMutex), DB: db}, nil
}

// Put a merkle proof of the block
func (db *ProofStore) Put(proof *bloom.MerkleProof) error {
	db.Lock()
	defer db.Unlock()

	return db.Update(func(tx *bolt.Tx) error {

		bytes, err := serializeProof(proof)
		if err != nil {
			return err
		}

		var key = make([]byte, 4)
		binary.LittleEndian.PutUint32(key[:], proof.Height)

		err = tx.Bucket(BKTProofs).Put(key, bytes)
		if err != nil {
			return err
		}

		return nil
	})
}

// Get a merkle proof of a block
func (db *ProofStore) Get(height uint32) (proof *bloom.MerkleProof, err error) {
	db.RLock()
	defer db.RUnlock()

	err = db.View(func(tx *bolt.Tx) error {

		var key = make([]byte, 4)
		binary.LittleEndian.PutUint32(key[:], height)

		proof, err = getProof(tx, key)
		if err != nil {
			return err
		}

		return nil
	})

	if err != nil {
		return nil, err
	}

	return proof, err
}

// Get all merkle proofs in database
func (db *ProofStore) GetAll() (proofs []*bloom.MerkleProof, err error) {
	db.RLock()
	defer db.RUnlock()

	err = db.View(func(tx *bolt.Tx) error {

		err := tx.Bucket(BKTProofs).ForEach(func(k, v []byte) error {

			proof, err := deserializeProof(v)
			if err != nil {
				return err
			}

			proofs = append(proofs, proof)

			return nil
		})

		if err != nil {
			return err
		}

		return nil
	})

	return proofs, nil
}

// Delete a merkle proof of a block
func (db *ProofStore) Delete(height uint32) error {
	db.Lock()
	defer db.Unlock()

	return db.Update(func(tx *bolt.Tx) error {
		var key = make([]byte, 4)
		binary.LittleEndian.PutUint32(key[:], height)

		return tx.Bucket(BKTProofs).Delete(key)
	})
}

func getProof(tx *bolt.Tx, key []byte) (*bloom.MerkleProof, error) {
	proofBytes := tx.Bucket(BKTProofs).Get(key)
	if proofBytes == nil {
		return nil, errors.New(fmt.Sprintf("MerkleProof %s does not exist in database", hex.EncodeToString(key)))
	}

	return deserializeProof(proofBytes)
}

func serializeProof(proof *bloom.MerkleProof) ([]byte, error) {
	buf := new(bytes.Buffer)
	err := proof.Serialize(buf)
	if err != nil {
		return nil, err
	}
	return buf.Bytes(), nil
}

func deserializeProof(body []byte) (*bloom.MerkleProof, error) {
	var proof bloom.MerkleProof
	err := proof.Deserialize(bytes.NewReader(body))
	if err != nil {
		return nil, err
	}
	return &proof, nil
}
