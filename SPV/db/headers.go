package db

import (
	"errors"
	"fmt"
	"sync"

	. "SPVWallet/core"
	"SPVWallet/log"

	"github.com/boltdb/bolt"
)

type Headers interface {
	// Add a new header to blockchain
	Add(header *Header) error

	// Get previous block of the given header
	GetPrevious(header *Header) (*Header, error)

	// Get full header with it's hash
	GetHeader(hash *Uint256) (*Header, error)

	// Get the header on chain tip
	GetTip() (*Header, error)

	// Rollback one block from chain tip
	Rollback() (*Header, error)

	// Close db
	Close()
}

// HeadersDB implements Headers using bolt DB
type HeadersDB struct {
	sync.RWMutex
	*bolt.DB
}

var (
	BKTHeaders  = []byte("Headers")
	BKTChainTip = []byte("ChainTip")
	KEYChainTip = []byte("ChainTip")
)

func NewHeadersDB() (Headers, error) {
	db, err := bolt.Open("headers.bin", 0644, &bolt.Options{InitialMmapSize: 5000000})
	if err != nil {
		return nil, err
	}

	db.Update(func(btx *bolt.Tx) error {
		_, err := btx.CreateBucketIfNotExists(BKTHeaders)
		if err != nil {
			return err
		}
		_, err = btx.CreateBucketIfNotExists(BKTChainTip)
		if err != nil {
			return err
		}
		return nil
	})

	return &HeadersDB{DB: db}, nil
}

// Add a new header to blockchain
func (db *HeadersDB) Add(header *Header) error {
	db.Lock()
	defer db.Unlock()

	log.Trace("Headers db add header:", header.Hash().String())
	return db.Update(func(tx *bolt.Tx) error {

		bytes, err := header.Bytes()
		if err != nil {
			return err
		}

		err = tx.Bucket(BKTHeaders).Put(header.Hash().Bytes(), bytes)
		if err != nil {
			return err
		}

		err = tx.Bucket(BKTChainTip).Put(KEYChainTip, bytes)
		if err != nil {
			return err
		}

		return nil
	})
}

// Get previous block of the given header
func (db *HeadersDB) GetPrevious(header *Header) (*Header, error) {
	return db.GetHeader(&header.Previous)
}

// Get full header with it's hash
func (db *HeadersDB) GetHeader(hash *Uint256) (header *Header, err error) {
	db.RLock()
	defer db.RUnlock()

	err = db.View(func(tx *bolt.Tx) error {

		header, err = getHeader(tx, hash)
		if err != nil {
			return err
		}

		return nil
	})

	if err != nil {
		return nil, err
	}

	return header, err
}

// Get the header on chain tip
func (db *HeadersDB) GetTip() (header *Header, err error) {
	db.RLock()
	defer db.RUnlock()

	err = db.View(func(tx *bolt.Tx) error {

		header, err = getTip(tx)
		if err != nil {
			return err
		}

		return nil
	})

	if err != nil {
		log.Error("Headers db get tip err,", err)
		return nil, err
	}

	return header, err
}

// Rollback one block from chain tip
func (db *HeadersDB) Rollback() (removed *Header, err error) {
	db.Lock()
	defer db.Unlock()

	err = db.View(func(tx *bolt.Tx) error {

		var newTip *Header
		var newTipBytes []byte

		removed, err = getTip(tx)
		if err != nil {
			return err
		}

		newTip, err = getHeader(tx, &removed.Previous)
		if err != nil {
			return err
		}

		err = tx.Bucket(BKTHeaders).Delete(removed.Hash().Bytes())
		if err != nil {
			return err
		}

		newTipBytes, err = newTip.Bytes()
		if err != nil {
			return err
		}

		return tx.Bucket(BKTChainTip).Put(KEYChainTip, newTipBytes)
	})

	if err != nil {
		log.Error("Headers db rollback err,", err)
		return nil, err
	}

	return removed, err
}

// Close db
func (db *HeadersDB) Close() {
	db.Lock()
	db.DB.Close()
	log.Info("Headers DB closed")
}

func getHeader(tx *bolt.Tx, hash *Uint256) (*Header, error) {
	headerBytes := tx.Bucket(BKTHeaders).Get(hash.Bytes())
	if headerBytes == nil {
		return nil, errors.New(fmt.Sprintf("Header %s does not exist in database", hash.String()))
	}

	return HeaderFromBytes(headerBytes)
}

func getTip(tx *bolt.Tx) (*Header, error) {
	headerBytes := tx.Bucket(BKTChainTip).Get(KEYChainTip)
	if headerBytes == nil {
		// No header exists in db, return empty header
		return new(Header), nil
	}

	return HeaderFromBytes(headerBytes)
}
