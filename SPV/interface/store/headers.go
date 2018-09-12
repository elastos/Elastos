package store

import (
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/boltdb/bolt"
	"github.com/cevaris/ordered_map"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

var (
	BKTHeaders    = []byte("Headers")
	BKTHeightHash = []byte("HeightHash")
	BKTChainTip   = []byte("ChainTip")
	KEYChainTip   = []byte("ChainTip")
)

// Ensure headers implement database.Headers interface.
var _ HeaderStore = (*headers)(nil)

type headers struct {
	*sync.RWMutex
	*bolt.DB
	cache *cache
}

func NewHeaderStore() (*headers, error) {
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

	headers := &headers{
		RWMutex: new(sync.RWMutex),
		DB:      db,
		cache:   newCache(100),
	}

	headers.initCache()

	return headers, nil
}

func (h *headers) initCache() {
	best, err := h.GetBest()
	if err != nil {
		return
	}
	h.cache.tip = best
	headers := []*util.Header{best}
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

func (h *headers) Put(header *util.Header, newTip bool) error {
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

func (h *headers) GetPrevious(header *util.Header) (*util.Header, error) {
	if header.Height == 1 {
		return &util.Header{TotalWork: new(big.Int)}, nil
	}
	return h.Get(&header.Previous)
}

func (h *headers) Get(hash *common.Uint256) (header *util.Header, err error) {
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

func (h *headers) GetBest() (header *util.Header, err error) {
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

func (h *headers) GetByHeight(height uint32) (header *util.Header, err error) {
	h.RLock()
	defer h.RUnlock()

	err = h.View(func(tx *bolt.Tx) error {
		var key [4]byte
		binary.LittleEndian.PutUint32(key[:], height)
		hashBytes := tx.Bucket(BKTHeightHash).Get(key[:])
		header, err = getHeader(tx, BKTHeaders, hashBytes)
		if err != nil {
			return err
		}
		return err
	})
	if err != nil {
		return header, fmt.Errorf("header not exist on height %d", height)
	}

	return header, err
}

func (h *headers) Clear() error {
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
func (h *headers) Close() error {
	h.Lock()
	defer h.Unlock()
	return h.DB.Close()
}

func getHeader(tx *bolt.Tx, bucket []byte, key []byte) (*util.Header, error) {
	headerBytes := tx.Bucket(bucket).Get(key)
	if headerBytes == nil {
		return nil, fmt.Errorf("header %s does not exist in database", hex.EncodeToString(key))
	}

	var header util.Header
	err := header.Deserialize(headerBytes)
	if err != nil {
		return nil, err
	}

	return &header, nil
}

type cache struct {
	sync.RWMutex
	size    int
	tip     *util.Header
	headers *ordered_map.OrderedMap
}

func newCache(size int) *cache {
	return &cache{
		size:    size,
		headers: ordered_map.NewOrderedMap(),
	}
}

func (cache *cache) pop() {
	iter := cache.headers.IterFunc()
	k, ok := iter()
	if ok {
		cache.headers.Delete(k.Key)
	}
}

func (cache *cache) Set(header *util.Header) {
	cache.Lock()
	defer cache.Unlock()

	if cache.headers.Len() > cache.size {
		cache.pop()
	}
	cache.headers.Set(header.Hash().String(), header)
}

func (cache *cache) Get(hash *common.Uint256) (*util.Header, error) {
	cache.RLock()
	defer cache.RUnlock()

	sh, ok := cache.headers.Get(hash.String())
	if !ok {
		return nil, errors.New("Header not found in cache ")
	}
	return sh.(*util.Header), nil
}
