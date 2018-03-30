package db

import (
	"database/sql"
	"sync"

	. "github.com/elastos/Elastos.ELA.SPV/core"
	"bytes"
)

const CreateProofsDB = `CREATE TABLE IF NOT EXISTS Proofs(
				BlockHash BLOB NOT NULL PRIMARY KEY,
				Height INTEGER NOT NULL,
				RawData BLOB NOT NULL
			);`

type ProofsDB struct {
	*sync.RWMutex
	*sql.DB
}

func NewProofsDB(db *sql.DB, lock *sync.RWMutex) (Proofs, error) {
	_, err := db.Exec(CreateProofsDB)
	if err != nil {
		return nil, err
	}
	return &ProofsDB{RWMutex: lock, DB: db}, nil
}

// Put a merkle proof of the block
func (db *ProofsDB) Put(proof *Proof) error {
	db.Lock()
	defer db.Unlock()

	stmt, err := db.Prepare("INSERT OR REPLACE INTO Proofs(BlockHash, Height, RawData) VALUES(?,?,?)")
	if err != nil {
		return err
	}
	defer stmt.Close()

	buf := new(bytes.Buffer)
	err = proof.Serialize(buf)
	if err != nil {
		return err
	}

	_, err = stmt.Exec(proof.BlockHash.Bytes(), proof.Height, buf.Bytes())
	if err != nil {
		return err
	}

	return nil
}

// Get a merkle proof of a block
func (db *ProofsDB) Get(blockHash *Uint256) (*Proof, error) {
	db.RLock()
	defer db.RUnlock()

	row := db.QueryRow("SELECT RawData FROM Proofs WHERE BlockHash=?", blockHash.Bytes())
	var rawData []byte
	err := row.Scan(&rawData)
	if err != nil {
		return nil, err
	}

	var proof Proof
	err = proof.Deserialize(bytes.NewReader(rawData))
	if err != nil {
		return nil, err
	}

	return &proof, nil
}

// Get all merkle proofs in database
func (db *ProofsDB) GetAll() ([]*Proof, error) {
	db.RLock()
	defer db.RUnlock()

	rows, err := db.Query("SELECT BlockHash, RawData FROM Proofs")
	if err != nil {
		return nil, err
	}

	var proofs []*Proof
	for rows.Next() {
		var rawData []byte
		err := rows.Scan(&rawData)
		if err != nil {
			return nil, err
		}

		var proof Proof
		err = proof.Deserialize(bytes.NewReader(rawData))
		if err != nil {
			return nil, err
		}
		proofs = append(proofs, &proof)
	}

	return proofs, nil
}

// Delete a merkle proof of a block
func (db *ProofsDB) Delete(blockHash *Uint256) error {
	db.Lock()
	defer db.Unlock()

	_, err := db.Exec("DELETE FROM Proofs WHERE BlockHash=?", blockHash.Bytes())
	if err != nil {
		return err
	}

	return nil
}
