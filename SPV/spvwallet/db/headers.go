package db

import (
	"errors"
	"encoding/hex"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/spvwallet/log"
	"github.com/cevaris/ordered_map"

	"github.com/boltdb/bolt"
	"math/big"
)

type Headers interface {
	// Add a new header to blockchain
	Put(header *Header, newTip bool) error

	// Get previous block of the given header
	GetPrevious(header *Header) (*Header, error)

	// Get full header with it's hash
	GetHeader(hash common.Uint256) (*Header, error)

	// Get the header on chain tip
	GetTip() (*Header, error)

	// Reset database, clear all data
	Reset() error

	// Close db
	Close()
}

// HeadersDB implements Headers using bolt DB
type HeadersDB struct {
	*sync.RWMutex
	*bolt.DB
	cache *HeaderCache
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

	headers := &HeadersDB{
		RWMutex: new(sync.RWMutex),
		DB:      db,
		cache:   newHeaderCache(100),
	}

	headers.initCache()

	return headers, nil
}

func (db *HeadersDB) initCache() {
	best, err := db.GetTip()
	if err != nil {
		return
	}
	db.cache.tip = best
	headers := []*Header{best}
	for i := 0; i < 99; i++ {
		sh, err := db.GetPrevious(best)
		if err != nil {
			break
		}
		headers = append(headers, sh)
	}
	for i := len(headers) - 1; i >= 0; i-- {
		db.cache.Set(headers[i])
	}
}

// Add a new header to blockchain
func (db *HeadersDB) Put(header *Header, newTip bool) error {
	db.Lock()
	defer db.Unlock()

	db.cache.Set(header)
	if newTip {
		db.cache.tip = header
	}
	return db.Update(func(tx *bolt.Tx) error {

		bytes, err := header.Serialize()
		if err != nil {
			return err
		}

		err = tx.Bucket(BKTHeaders).Put(header.Hash().Bytes(), bytes)
		if err != nil {
			return err
		}

		if newTip {
			err = tx.Bucket(BKTChainTip).Put(KEYChainTip, bytes)
			if err != nil {
				return err
			}
		}

		return nil
	})
}

// Get previous block of the given header
func (db *HeadersDB) GetPrevious(header *Header) (*Header, error) {
	if header.Height == 1 {
		return &Header{TotalWork: new(big.Int)}, nil
	}
	return db.GetHeader(header.Previous)
}

// Get full header with it's hash
func (db *HeadersDB) GetHeader(hash common.Uint256) (header *Header, err error) {
	db.RLock()
	defer db.RUnlock()

	header, err = db.cache.Get(hash)
	if err == nil {
		return header, nil
	}

	err = db.View(func(tx *bolt.Tx) error {

		header, err = getHeader(tx, BKTHeaders, hash.Bytes())
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

	if db.cache.tip != nil {
		return db.cache.tip, nil
	}

	err = db.View(func(tx *bolt.Tx) error {

		header, err = getHeader(tx, BKTChainTip, KEYChainTip)
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

func (db *HeadersDB) Reset() error {
	db.Lock()
	defer db.Unlock()

	return db.Update(func(tx *bolt.Tx) error {
		err := tx.DeleteBucket(BKTHeaders)
		if err != nil {
			return err
		}

		return tx.DeleteBucket(BKTChainTip)
	})
}

// Close db
func (db *HeadersDB) Close() {
	db.Lock()
	db.DB.Close()
	log.Debug("Headers DB closed")
}

func getHeader(tx *bolt.Tx, bucket []byte, key []byte) (*Header, error) {
	headerBytes := tx.Bucket(bucket).Get(key)
	if headerBytes == nil {
		return nil, errors.New(fmt.Sprintf("Header %s does not exist in database", hex.EncodeToString(key)))
	}

	var header Header
	err := header.Deserialize(headerBytes)
	if err != nil {
		return nil, err
	}

	return &header, nil
}

type HeaderCache struct {
	sync.RWMutex
	size    int
	tip     *Header
	headers *ordered_map.OrderedMap
}

func newHeaderCache(size int) *HeaderCache {
	return &HeaderCache{
		size:    size,
		headers: ordered_map.NewOrderedMap(),
	}
}

func (cache *HeaderCache) pop() {
	iter := cache.headers.IterFunc()
	k, ok := iter()
	if ok {
		cache.headers.Delete(k.Key)
	}
}

func (cache *HeaderCache) Set(header *Header) {
	cache.Lock()
	defer cache.Unlock()

	if cache.headers.Len() > cache.size {
		cache.pop()
	}
	cache.headers.Set(header.Hash().String(), header)
}

func (cache *HeaderCache) Get(hash common.Uint256) (*Header, error) {
	cache.RLock()
	defer cache.RUnlock()

	sh, ok := cache.headers.Get(hash.String())
	if !ok {
		return nil, errors.New("Header not found in cache ")
	}
	return sh.(*Header), nil
}
