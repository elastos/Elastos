package store

import (
	"encoding/binary"
	"encoding/hex"
	"errors"
	"fmt"
	"path/filepath"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/cevaris/ordered_map"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
)

var (
	BKTHeaders  = []byte("H")
	BKTIndexes  = []byte("I")
	BKTChainTip = []byte("B")
)

// Ensure headers implement database.Headers interface.
var _ HeaderStore = (*headers)(nil)

type headers struct {
	*sync.RWMutex
	db        *leveldb.DB
	cache     *cache
	newHeader func() util.BlockHeader
}

func NewHeaderStore(dataDir string, newHeader func() util.BlockHeader) (*headers, error) {
	db, err := leveldb.OpenFile(filepath.Join(dataDir, "header"), nil)
	if err != nil {
		return nil, err
	}

	headers := &headers{
		RWMutex:   new(sync.RWMutex),
		db:        db,
		cache:     newCache(100),
		newHeader: newHeader,
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
		h.cache.set(headers[i])
	}
}

func (h *headers) Put(header *util.Header, newTip bool) error {
	h.Lock()
	defer h.Unlock()

	h.cache.set(header)
	if newTip {
		h.cache.tip = header
	}
	key := toKey(BKTHeaders, header.Hash().Bytes()...)

	bytes, err := header.Serialize()
	if err != nil {
		return err
	}

	err = h.db.Put(key, bytes, nil)
	if err != nil {
		return err
	}

	if newTip {
		err = h.db.Put(BKTChainTip, bytes, nil)
		if err != nil {
			return err
		}
	}

	var height [4]byte
	binary.LittleEndian.PutUint32(height[:], header.Height)
	index := toKey(BKTIndexes, height[:]...)
	return h.db.Put(index, header.Hash().Bytes(), nil)
}

func (h *headers) GetPrevious(header *util.Header) (*util.Header, error) {
	hash := header.Previous()
	return h.Get(&hash)
}

func (h *headers) Get(hash *common.Uint256) (header *util.Header, err error) {
	h.RLock()
	defer h.RUnlock()

	header, err = h.cache.get(hash)
	if err == nil {
		return header, nil
	}

	return h.getHeader(toKey(BKTHeaders, hash.Bytes()...))
}

func (h *headers) GetBest() (header *util.Header, err error) {
	h.RLock()
	defer h.RUnlock()

	if h.cache.tip != nil {
		return h.cache.tip, nil
	}

	return h.getHeader(BKTChainTip)
}

func (h *headers) GetByHeight(height uint32) (header *util.Header, err error) {
	h.RLock()
	defer h.RUnlock()

	var key [4]byte
	binary.LittleEndian.PutUint32(key[:], height)
	hashBytes, err := h.db.Get(toKey(BKTIndexes, key[:]...), nil)
	if err != nil {
		return nil, err
	}

	header, err = h.getHeader(toKey(BKTHeaders, hashBytes...))
	if err != nil {
		return nil, err
	}

	return header, err
}

func (h *headers) Clear() error {
	h.Lock()
	defer h.Unlock()

	batch := new(leveldb.Batch)
	inter := h.db.NewIterator(nil, nil)
	for inter.Next() {
		batch.Delete(inter.Key())
	}
	inter.Release()
	return h.db.Write(batch, nil)
}

// Close db
func (h *headers) Close() error {
	h.Lock()
	defer h.Unlock()
	return h.db.Close()
}

func (h *headers) getHeader(key []byte) (*util.Header, error) {
	data, err := h.db.Get(key, nil)
	if err != nil {
		return nil, fmt.Errorf("header %s does not exist in database",
			hex.EncodeToString(key))
	}

	var header util.Header
	header.BlockHeader = h.newHeader()
	err = header.Deserialize(data)
	if err != nil {
		return nil, err
	}

	return &header, nil
}

type cache struct {
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

func (cache *cache) set(header *util.Header) {
	if cache.headers.Len() > cache.size {
		cache.pop()
	}
	cache.headers.Set(header.Hash().String(), header)
}

func (cache *cache) get(hash *common.Uint256) (*util.Header, error) {
	sh, ok := cache.headers.Get(hash.String())
	if !ok {
		return nil, errors.New("Header not found in cache ")
	}
	return sh.(*util.Header), nil
}

func toKey(bucket []byte, index ...byte) []byte {
	return append(bucket, index...)
}

func subKey(bucket []byte, key []byte) []byte {
	return key[len(bucket):]
}
