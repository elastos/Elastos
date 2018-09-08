package headers

import (
	"encoding/hex"
	"errors"
	"fmt"
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure HeadersDB implement headers interface
var _ database.Headers = (*HeadersDB)(nil)

// Headers implements Headers using bolt DB
type HeadersDB struct {
	*sync.RWMutex
	*bolt.DB
	cache *cache
}

var (
	BKTHeaders  = []byte("Headers")
	BKTChainTip = []byte("ChainTip")
	KEYChainTip = []byte("ChainTip")
)

func NewHeadersDB() (*HeadersDB, error) {
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
		h.cache.set(headers[i])
	}
}

func (h *HeadersDB) Put(header *util.Header, newTip bool) error {
	h.Lock()
	defer h.Unlock()

	h.cache.set(header)
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

func (h *HeadersDB) GetPrevious(header *util.Header) (*util.Header, error) {
	if header.Height == 1 {
		return &util.Header{TotalWork: new(big.Int)}, nil
	}
	return h.Get(&header.Previous)
}

func (h *HeadersDB) Get(hash *common.Uint256) (header *util.Header, err error) {
	h.RLock()
	defer h.RUnlock()

	header, err = h.cache.get(hash)
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

func (h *HeadersDB) GetBest() (header *util.Header, err error) {
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
		return nil, fmt.Errorf("Headers db get tip error %s", err.Error())
	}

	return header, err
}

func (h *HeadersDB) Clear() error {
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
func (h *HeadersDB) Close() error {
	h.Lock()
	err := h.DB.Close()
	log.Debug("Headers DB closed")
	return err
}

func getHeader(tx *bolt.Tx, bucket []byte, key []byte) (*util.Header, error) {
	headerBytes := tx.Bucket(bucket).Get(key)
	if headerBytes == nil {
		return nil, errors.New(fmt.Sprintf("Header %s does not exist in database", hex.EncodeToString(key)))
	}

	var header util.Header
	err := header.Deserialize(headerBytes)
	if err != nil {
		return nil, err
	}

	return &header, nil
}
