package headers

import (
	"errors"
	"sync"

	"github.com/cevaris/ordered_map"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

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

func (cache *cache) set(header *util.Header) {
	cache.Lock()
	defer cache.Unlock()

	if cache.headers.Len() > cache.size {
		cache.pop()
	}
	cache.headers.Set(header.Hash().String(), header)
}

func (cache *cache) get(hash *common.Uint256) (*util.Header, error) {
	cache.RLock()
	defer cache.RUnlock()

	sh, ok := cache.headers.Get(hash.String())
	if !ok {
		return nil, errors.New("Header not found in cache ")
	}
	return sh.(*util.Header), nil
}
