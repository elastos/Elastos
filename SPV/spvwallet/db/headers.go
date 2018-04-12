package db

import (
	"errors"
	"encoding/hex"
	"fmt"
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/common"
	"github.com/elastos/Elastos.ELA.SPV/db"
	"github.com/elastos/Elastos.ELA.SPV/log"

	"github.com/boltdb/bolt"
	"github.com/cevaris/ordered_map"
)

type Headers interface {
	// Add a new header to blockchain
	Put(header *db.StoreHeader, newTip bool) error

	// Get previous block of the given header
	GetPrevious(header *db.StoreHeader) (*db.StoreHeader, error)

	// Get full header with it's hash
	GetHeader(hash common.Uint256) (*db.StoreHeader, error)

	// Get the header on chain tip
	GetTip() (*db.StoreHeader, error)

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

func (h *HeadersDB) initCache() {
	best, err := h.GetTip()
	if err != nil {
		return
	}
	h.cache.tip = best
	headers := []*db.StoreHeader{best}
	for i := 0; i < 99; i++ {
		sh, err := h.GetPrevious(best)
		if err != nil {
			break
		}
		headers = append(headers, sh)
	}
	for i := len(headers) - 1; i >= 0; i-- {
		h.cache.Set(headers[i])
	}
}

// Add a new header to blockchain
func (h *HeadersDB) Put(header *db.StoreHeader, newTip bool) error {
	h.Lock()
	defer h.Unlock()

	h.cache.Set(header)
	if newTip {
		h.cache.tip = header
	}
	return h.Update(func(tx *bolt.Tx) error {

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
func (h *HeadersDB) GetPrevious(header *db.StoreHeader) (*db.StoreHeader, error) {
	if header.Height == 1 {
		return &db.StoreHeader{TotalWork: new(big.Int)}, nil
	}
	return h.GetHeader(header.Previous)
}

// Get full header with it's hash
func (h *HeadersDB) GetHeader(hash common.Uint256) (header *db.StoreHeader, err error) {
	h.RLock()
	defer h.RUnlock()

	header, err = h.cache.Get(hash)
	if err == nil {
		return header, nil
	}

	err = h.View(func(tx *bolt.Tx) error {

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
func (h *HeadersDB) GetTip() (header *db.StoreHeader, err error) {
	h.RLock()
	defer h.RUnlock()

	if h.cache.tip != nil {
		return h.cache.tip, nil
	}

	err = h.View(func(tx *bolt.Tx) error {

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

func (h *HeadersDB) Reset() error {
	h.Lock()
	defer h.Unlock()

	return h.Update(func(tx *bolt.Tx) error {
		err := tx.DeleteBucket(BKTHeaders)
		if err != nil {
			return err
		}

		return tx.DeleteBucket(BKTChainTip)
	})
}

// Close db
func (h *HeadersDB) Close() {
	h.Lock()
	h.DB.Close()
	log.Debug("Headers DB closed")
}

func getHeader(tx *bolt.Tx, bucket []byte, key []byte) (*db.StoreHeader, error) {
	headerBytes := tx.Bucket(bucket).Get(key)
	if headerBytes == nil {
		return nil, errors.New(fmt.Sprintf("Header %s does not exist in database", hex.EncodeToString(key)))
	}

	var header db.StoreHeader
	err := header.Deserialize(headerBytes)
	if err != nil {
		return nil, err
	}

	return &header, nil
}

type HeaderCache struct {
	sync.RWMutex
	size    int
	tip     *db.StoreHeader
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

func (cache *HeaderCache) Set(header *db.StoreHeader) {
	cache.Lock()
	defer cache.Unlock()

	if cache.headers.Len() > cache.size {
		cache.pop()
	}
	cache.headers.Set(header.Hash().String(), header)
}

func (cache *HeaderCache) Get(hash common.Uint256) (*db.StoreHeader, error) {
	cache.RLock()
	defer cache.RUnlock()

	sh, ok := cache.headers.Get(hash.String())
	if !ok {
		return nil, errors.New("Header not found in cache ")
	}
	return sh.(*db.StoreHeader), nil
}
