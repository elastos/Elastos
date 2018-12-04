package db

import (
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/store"
	"github.com/elastos/Elastos.ELA.Utility/common"

	"github.com/boltdb/bolt"
	"github.com/cevaris/ordered_map"
)

var (
	BKTHeaders    = []byte("Headers")
	BKTHeightHash = []byte("HeightHash")
	BKTChainTip   = []byte("ChainTip")
	KEYChainTip   = []byte("ChainTip")
)

type HeaderStore struct {
	*sync.RWMutex
	*bolt.DB
	cache *HeaderCache
}

func NewHeaderStore() (*HeaderStore, error) {
	db, err := bolt.Open("headers.bin", 0644, &bolt.Options{InitialMmapSize: 5000000})
	if err != nil {
		return nil, err
	}

	db.Update(func(btx *bolt.Tx) error {
		_, err := btx.CreateBucketIfNotExists(BKTHeaders)
		if err != nil {
			return err
		}
		_, err = btx.CreateBucketIfNotExists(BKTHeightHash)
		if err != nil {
			return err
		}
		_, err = btx.CreateBucketIfNotExists(BKTChainTip)
		if err != nil {
			return err
		}
		return nil
	})

	headers := &HeaderStore{
		RWMutex: new(sync.RWMutex),
		DB:      db,
		cache:   newHeaderCache(100),
	}

	headers.initCache()

	return headers, nil
}

func (h *HeaderStore) initCache() {
	best, err := h.GetBestHeader()
	if err != nil {
		return
	}
	h.cache.tip = best
	headers := []*store.StoreHeader{best}
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

func (h *HeaderStore) PutHeader(header *store.StoreHeader, newTip bool) error {
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

		var key [4]byte
		binary.LittleEndian.PutUint32(key[:], header.Height)
		return tx.Bucket(BKTHeightHash).Put(key[:], header.Hash().Bytes())
	})
}

func (h *HeaderStore) GetPrevious(header *store.StoreHeader) (*store.StoreHeader, error) {
	if header.Height == 1 {
		return &store.StoreHeader{TotalWork: new(big.Int)}, nil
	}
	return h.GetHeader(&header.Previous)
}

func (h *HeaderStore) GetHeader(hash *common.Uint256) (header *store.StoreHeader, err error) {
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

	return header, err
}

func (h *HeaderStore) GetBestHeader() (header *store.StoreHeader, err error) {
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

	return header, err
}

func (h *HeaderStore) GetHeaderHash(height uint32) (hash *common.Uint256, err error) {
	h.RLock()
	defer h.RUnlock()

	err = h.View(func(tx *bolt.Tx) error {
		var key [4]byte
		binary.LittleEndian.PutUint32(key[:], height)
		data := tx.Bucket(BKTHeightHash).Get(key[:])
		hash, err = common.Uint256FromBytes(data)
		return err
	})

	if err != nil {
		return hash, fmt.Errorf("header hash not exist on height %d", height)
	}

	return hash, err
}

func (h *HeaderStore) Reset() error {
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
func (h *HeaderStore) Close() {
	h.Lock()
	h.DB.Close()
}

func getHeader(tx *bolt.Tx, bucket []byte, key []byte) (*store.StoreHeader, error) {
	headerBytes := tx.Bucket(bucket).Get(key)
	if headerBytes == nil {
		return nil, fmt.Errorf("header %s does not exist in database", hex.EncodeToString(key))
	}

	var header store.StoreHeader
	err := header.Deserialize(headerBytes)
	if err != nil {
		return nil, err
	}

	return &header, nil
}

type HeaderCache struct {
	sync.RWMutex
	size    int
	tip     *store.StoreHeader
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

func (cache *HeaderCache) Set(header *store.StoreHeader) {
	cache.Lock()
	defer cache.Unlock()

	if cache.headers.Len() > cache.size {
		cache.pop()
	}
	cache.headers.Set(header.Hash().String(), header)
}

func (cache *HeaderCache) Get(hash *common.Uint256) (*store.StoreHeader, error) {
	cache.RLock()
	defer cache.RUnlock()

	sh, ok := cache.headers.Get(hash.String())
	if !ok {
		return nil, errors.New("Header not found in cache ")
	}
	return sh.(*store.StoreHeader), nil
}
