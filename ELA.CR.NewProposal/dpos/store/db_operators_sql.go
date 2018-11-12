package store

import (
	"database/sql"
	"fmt"
	_ "github.com/mattn/go-sqlite3"

	"github.com/elastos/Elastos.ELA/dpos/log"
)

type SqlDBOperator struct {
	db *sql.DB

	dbDriverName string

	dbFilePath string
}

func (s *SqlDBOperator) InitConnection(connParams ...interface{}) {
	//todo complete me
	driverName, ok := connParams[0].(string)
	if !ok {
		log.Error("[InitConnection] Invalid sql db driver name.")
		return
	}
	s.dbDriverName = driverName

	dbFilePath, ok := connParams[1].(string)
	if !ok {
		log.Error("[InitConnection] Invalid sql db file path.")
	}
	s.dbFilePath = dbFilePath
}

func (s *SqlDBOperator) Connect() error {
	db, err := sql.Open(s.dbDriverName, s.dbFilePath)
	if err != nil {
		fmt.Println("[Connect()]  open database error: ", err)
		return err
	}
	s.db = db
	return nil
}

func (s *SqlDBOperator) Disconnect() error {

	err := s.db.Close()
	if err != nil {
		fmt.Println("[Disconnect()] close database error: ", err)
	}
	return err
}

func (s *SqlDBOperator) Execute(script string, params ...interface{}) (uint64, error) {
	log.Debug("[Execute] script:", script)
	log.Debug("[Execute] params:", params)
	result, err := s.db.Exec(script, params...)
	if err != nil {
		fmt.Println("[Execute(script,params...)] db exec error: ", err)
		return 0, err
	}

	lastid, err := result.LastInsertId()
	if err != nil {
		fmt.Println("[Execute(script,params...)] result lastInsertId error: ", err)
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
		fmt.Println("[Query]  row scan error: ", err)
		return 0, err
	}
	fmt.Println("[Query] result = ", result)
	return uint64(result), nil
}
