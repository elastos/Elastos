package store

import (
	"database/sql"
	_ "github.com/mattn/go-sqlite3"

	"errors"
	"github.com/elastos/Elastos.ELA/dpos/log"
)

type SqlDBOperator struct {
	db *sql.DB

	dbDriverName string

	dbFilePath string
}

func (s *SqlDBOperator) InitConnection(connParams ...interface{}) error {
	driverName, ok := connParams[0].(string)
	if !ok {
		return errors.New("[InitConnection] Invalid sql db driver name.")
	}
	s.dbDriverName = driverName

	dbFilePath, ok := connParams[1].(string)
	if !ok {
		return errors.New("[InitConnection] Invalid sql db file path.")
	}
	s.dbFilePath = dbFilePath
	return nil
}

func (s *SqlDBOperator) Connect() error {
	db, err := sql.Open(s.dbDriverName, s.dbFilePath)
	if err != nil {
		log.Error("[Connect]  open database error:", err.Error())
		return err
	}
	s.db = db
	return nil
}

func (s *SqlDBOperator) Disconnect() error {
	err := s.db.Close()
	if err != nil {
		log.Error("[Disconnect] error:", err.Error())
		return err
	}
	return nil
}

func (s *SqlDBOperator) Execute(script string, params ...interface{}) (uint64, error) {
	log.Debug("[Execute] script:", script)
	log.Debug("[Execute] params:", params)
	result, err := s.db.Exec(script, params...)
	if err != nil {
		return 0, err
	}

	lastid, err := result.LastInsertId()
	if err != nil {
		return 0, err
	}
	return uint64(lastid), nil

}

func (s *SqlDBOperator) Query(script string, params ...interface{}) (uint64, error) {
	log.Debug("[Query] script:", script)
	log.Debug("[Query] params:", params)
	row := s.db.QueryRow(script, params...)

	var result int64
	err := row.Scan(&result)
	if err != nil {
		return 0, err
	}
	return uint64(result), nil
}
