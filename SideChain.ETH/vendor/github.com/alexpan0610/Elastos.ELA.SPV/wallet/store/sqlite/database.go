package sqlite

import (
	"database/sql"
	"fmt"
	"path/filepath"
	"sync"

	_ "github.com/mattn/go-sqlite3"
)

const (
	DriverName = "sqlite3"
)

// Ensure database implement DataStore interface
var _ DataStore = (*database)(nil)

type database struct {
	*sync.RWMutex
	*sql.DB

	state *state
	addrs *addrs
	txs   *txs
	utxos *utxos
	stxos *stxos
}

func NewDatabase(dataDir string) (*database, error) {
	db, err := sql.Open(DriverName, filepath.Join(dataDir, "wallet.db"))
	if err != nil {
		fmt.Println("Open sqlite db error:", err)
		return nil, err
	}
	// Use the same lock
	lock := new(sync.RWMutex)

	// Create state db
	state, err := NewState(db, lock)
	if err != nil {
		return nil, err
	}
	// Create addrs db
	addrs, err := NewAddrs(db, lock)
	if err != nil {
		return nil, err
	}
	// Create UTXOs db
	utxos, err := NewUTXOs(db, lock)
	if err != nil {
		return nil, err
	}
	// Create STXOs db
	stxos, err := NewSTXOs(db, lock)
	if err != nil {
		return nil, err
	}
	// Create Txs db
	txns, err := NewTxs(db, lock)
	if err != nil {
		return nil, err
	}

	return &database{
		RWMutex: lock,
		DB:      db,

		state: state,
		addrs: addrs,
		utxos: utxos,
		stxos: stxos,
		txs:   txns,
	}, nil
}

func (d *database) State() State {
	return d.state
}

func (d *database) Addrs() Addrs {
	return d.addrs
}

func (d *database) Txs() Txs {
	return d.txs
}

func (d *database) UTXOs() UTXOs {
	return d.utxos
}

func (d *database) STXOs() STXOs {
	return d.stxos
}

func (d *database) Batch() DataBatch {
	d.Lock()
	defer d.Unlock()

	tx, err := d.Begin()
	if err != nil {
		panic(err)
	}

	return &dataBatch{
		RWMutex: d.RWMutex,
		Tx:      tx,
	}
}

func (d *database) Clear() error {
	tx, err := d.Begin()
	if err != nil {
		return err
	}

	// Drop all tables except Addrs
	_, err = tx.Exec(`DROP TABLE IF EXISTS State;
							DROP TABLE IF EXISTS UTXOs;
							DROP TABLE IF EXISTS STXOs;
							DROP TABLE IF EXISTS TXNs;`)
	if err != nil {
		return err
	}

	return tx.Commit()
}

func (d *database) Close() error {
	d.Lock()
	err := d.DB.Close()
	log.Debug("sqlite database closed")
	return err
}
