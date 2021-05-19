package headers

import (
	"encoding/hex"
	"fmt"
	"path/filepath"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/util"
	"github.com/elastos/Elastos.ELA.SPV/wallet/sutil"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/syndtr/goleveldb/leveldb"
)

// Ensure Database implement headers interface
var _ database.Headers = (*Database)(nil)

// Headers implements Headers using bolt DB
type Database struct {
	*sync.RWMutex
	db        *leveldb.DB
	cache     *cache
	newHeader func() util.BlockHeader
}

var (
	BKTHeaders  = []byte("H")
	BKTChainTip = []byte("B")
)

func NewDatabase(dataDir string) (*Database, error) {
	db, err := leveldb.OpenFile(filepath.Join(dataDir, "header"), nil)
	if err != nil {
		return nil, err
	}

	headers := &Database{
		RWMutex:   new(sync.RWMutex),
		db:        db,
		cache:     newCache(100),
		newHeader: sutil.NewEmptyHeader,
	}

	headers.initCache()

	return headers, nil
}

func (d *Database) initCache() {
	best, err := d.GetBest()
	if err != nil {
		return
	}
	d.cache.tip = best
	headers := []*util.Header{best}
	for i := 0; i < 99; i++ {
		sh, err := d.GetPrevious(best)
		if err != nil {
			break
		}
		headers = append(headers, sh)
	}
	for i := len(headers) - 1; i >= 0; i-- {
		d.cache.set(headers[i])
	}
}

func (d *Database) Put(header *util.Header, newTip bool) error {
	d.Lock()
	defer d.Unlock()

	d.cache.set(header)
	if newTip {
		d.cache.tip = header
	}
	key := toKey(BKTHeaders, header.Hash().Bytes()...)

	bytes, err := header.Serialize()
	if err != nil {
		return err
	}

	err = d.db.Put(key, bytes, nil)
	if err != nil {
		return err
	}

	if newTip {
		err = d.db.Put(BKTChainTip, bytes, nil)
		if err != nil {
			return err
		}
	}
	return nil
}

func (d *Database) GetPrevious(header *util.Header) (*util.Header, error) {
	hash := header.Previous()
	return d.Get(&hash)
}

func (d *Database) Get(hash *common.Uint256) (header *util.Header, err error) {
	d.RLock()
	defer d.RUnlock()

	header, err = d.cache.get(hash)
	if err == nil {
		return header, nil
	}

	return d.getHeader(toKey(BKTHeaders, hash.Bytes()...))
}

func (d *Database) GetBest() (header *util.Header, err error) {
	d.RLock()
	defer d.RUnlock()

	if d.cache.tip != nil {
		return d.cache.tip, nil
	}

	return d.getHeader(BKTChainTip)
}

func (d *Database) Clear() error {
	d.Lock()
	defer d.Unlock()

	batch := new(leveldb.Batch)
	inter := d.db.NewIterator(nil, nil)
	for inter.Next() {
		batch.Delete(inter.Key())
	}
	inter.Release()
	return d.db.Write(batch, nil)
}

// Close db
func (d *Database) Close() error {
	d.Lock()
	defer d.Unlock()
	return d.db.Close()
}

func (d *Database) getHeader(key []byte) (*util.Header, error) {
	data, err := d.db.Get(key, nil)
	if err != nil {
		return nil, fmt.Errorf("header %s does not exist in database",
			hex.EncodeToString(key))
	}

	var header util.Header
	header.BlockHeader = d.newHeader()
	err = header.Deserialize(data)
	if err != nil {
		return nil, err
	}

	return &header, nil
}

func toKey(bucket []byte, index ...byte) []byte {
	return append(bucket, index...)
}
