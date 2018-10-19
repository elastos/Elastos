package headers

import (
	"encoding/hex"
	"fmt"
	"math/big"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/database"
	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/boltdb/bolt"
	"github.com/elastos/Elastos.ELA.Utility/common"
)

// Ensure Database implement headers interface
var _ database.Headers = (*Database)(nil)

// Headers implements Headers using bolt DB
type Database struct {
	*sync.RWMutex
	*bolt.DB
	cache          *cache
	newBlockHeader func() util.BlockHeader
}

var (
	BKTHeaders  = []byte("Headers")
	BKTChainTip = []byte("ChainTip")
	KEYChainTip = []byte("ChainTip")
)

func NewDatabase(newBlockHeader func() util.BlockHeader) (*Database, error) {
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

	headers := &Database{
		RWMutex:        new(sync.RWMutex),
		DB:             db,
		cache:          newHeaderCache(100),
		newBlockHeader: newBlockHeader,
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
	return d.Update(func(tx *bolt.Tx) error {

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

func (d *Database) GetPrevious(header *util.Header) (*util.Header, error) {
	if header.Height == 0 {
		return nil, fmt.Errorf("no more previous header")
	}
	if header.Height == 1 {
		return &util.Header{TotalWork: new(big.Int)}, nil
	}
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

	err = d.View(func(tx *bolt.Tx) error {

		header, err = d.getHeader(tx, BKTHeaders, hash.Bytes())
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

func (d *Database) GetBest() (header *util.Header, err error) {
	d.RLock()
	defer d.RUnlock()

	if d.cache.tip != nil {
		return d.cache.tip, nil
	}

	err = d.View(func(tx *bolt.Tx) error {
		header, err = d.getHeader(tx, BKTChainTip, KEYChainTip)
		return err
	})
	if err != nil {
		return nil, fmt.Errorf("Headers db get tip error %s", err.Error())
	}

	return header, err
}

func (d *Database) Clear() error {
	d.Lock()
	defer d.Unlock()

	return d.Update(func(tx *bolt.Tx) error {
		err := tx.DeleteBucket(BKTHeaders)
		if err != nil {
			return err
		}

		return tx.DeleteBucket(BKTChainTip)
	})
}

// Close db
func (d *Database) Close() error {
	d.Lock()
	err := d.DB.Close()
	log.Debug("headers database closed")
	return err
}

func (d *Database) getHeader(tx *bolt.Tx, bucket []byte, key []byte) (*util.Header, error) {
	headerBytes := tx.Bucket(bucket).Get(key)
	if headerBytes == nil {
		return nil, fmt.Errorf("Header %s does not exist in database", hex.EncodeToString(key))
	}

	var header util.Header
	header.BlockHeader = d.newBlockHeader()
	err := header.Deserialize(headerBytes)
	if err != nil {
		return nil, err
	}

	return &header, nil
}
