package store

import (
	"bytes"
	"errors"
	"fmt"
	_ "github.com/mattn/go-sqlite3"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA/blockchain"
)

type LevelDBOperator struct {
	blockchain.IStore

	dbFilePath string
}

func (s *LevelDBOperator) InitConnection(connParams ...interface{}) error {
	filePath, ok := connParams[0].(string)
	if !ok {
		return errors.New("[InitConnection] Invalid sql db file path.")
	}

	s.dbFilePath = filePath

	return nil
}

func (s *LevelDBOperator) Connect() error {
	db, err := blockchain.NewLevelDB(s.dbFilePath)
	if err != nil {
		return err
	}

	s.IStore = db
	return nil
}

func (s *LevelDBOperator) Disconnect() error {
	s.Close()
	return nil
}

func (s *LevelDBOperator) Create(table *DposTable) error {
	buf := new(bytes.Buffer)
	if err := table.Serialize(buf); err != nil {
		return err
	}

	key := getTableKey(table.Name)
	_, err := s.Get(key)
	if err != nil {
		return fmt.Errorf("already exist table: %s", table.Name)
	}

	if err := s.Put(getTableIDKey(table.Name), uint64ToBytes(uint64(0))); err != nil {
		return err
	}

	return s.Put(key, buf.Bytes())
}

func (s *LevelDBOperator) Insert(table *DposTable, fields []*Field) (uint64, error) {
	s.NewBatch()
	tableName := table.Name
	var hasPrimaryKey bool
	for _, f := range fields {
		if table.Column(f.Name) == table.PrimaryKey {
			hasPrimaryKey = true
		}
	}
	if !hasPrimaryKey {
		return 0, errors.New("have no primary key")
	}

	// key: tableName_rowID
	// value: [colvalue1, colvalue2, colvalue3, ...]
	idBytes, err := s.Get(getTableIDKey(table.Name))
	if err != nil {
		return 0, err
	}
	id := bytesToUint64(idBytes)
	rowID := id + 1
	data, err := table.Data(fields)
	if err != nil {
		return 0, err
	}
	s.BatchPut(getRowKey(tableName, rowID), data)

	for _, f := range fields {
		col := table.Column(f.Name)
		if col == table.PrimaryKey {
			buf := new(bytes.Buffer)
			common.WriteUint64(buf, rowID)
			key := getIndexKey(tableName, table.PrimaryKey, f.Data())
			if _, err := s.Get(key); err != nil {
				return 0, errors.New("duplicated primary")
			}
			// key: tableName_PrimaryKey_ColumnValue
			// value: rowID
			s.BatchPut(key, buf.Bytes())
		}
		for _, index := range table.Indexes {
			if col == index {
				key := getIndexKey(tableName, col, f.Data())
				var indexes []uint64
				indexData, err := s.Get(key)
				if err == nil {
					indexes, err = bytesToUint64List(indexData)
					if err != nil {
						return 0, err
					}
				}
				indexes = append(indexes, rowID)
				// key: tableName_IndexID_ColumnValue
				// value: [rowID1,rowID2,rowID3,...]
				indexListBytes, err := uint64ListToBytes(indexes)
				if err != nil {
					return 0, err
				}
				s.BatchPut(key, indexListBytes)
			}
		}
	}

	// update id
	s.BatchPut(getTableIDKey(table.Name), uint64ToBytes(rowID))
	if err := s.BatchCommit(); err != nil {
		return 0, err
	}
	return rowID, nil
}

func (s *LevelDBOperator) Select(table *DposTable, inputFields []*Field) ([][]*Field, error) {
	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}

	return s.selectValuesFromRowIDs(table, ids)
}

func (s *LevelDBOperator) SelectID(table *DposTable, inputFields []*Field) ([]uint64, error) {
	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *LevelDBOperator) selectValuesFromRowIDs(table *DposTable, rowIDs []uint64) ([][]*Field, error) {
	var result [][]*Field
	for _, rowID := range rowIDs {
		columnsData, err := s.Get(getRowKey(table.Name, rowID))
		if err != nil {
			return nil, err
		}
		fields, err := table.GetFields(columnsData)
		if err != nil {
			return nil, err
		}
		result = append(result, fields)
	}

	return result, nil
}

func (s *LevelDBOperator) selectRowIDs(table *DposTable, inputFields []*Field) ([]uint64, error) {
	idsCount := make(map[uint64]uint32)
	for _, f := range inputFields {
		rowIDs, err := s.selectRowsByField(table, f)
		if err != nil {
			return nil, err
		}
		for _, id := range rowIDs {
			idsCount[id] = idsCount[id] + 1
		}
	}
	var selectRowIDs []uint64
	for id, count := range idsCount {
		if count == uint32(len(inputFields)) {
			selectRowIDs = append(selectRowIDs, id)
		}
	}
	return selectRowIDs, nil
}

// because if one field is neither primary key nor index key, requires full table lookup
// only sport select from primary key or index column
func (s *LevelDBOperator) selectRowsByField(table *DposTable, inputField *Field) ([]uint64, error) {
	col := table.Column(inputField.Name)
	if col == table.PrimaryKey {
		rowIDBytes, err := s.Get(getIndexKey(table.Name, col, inputField.Data()))
		if err == nil {
			return nil, err
		}
		rowID := bytesToUint64(rowIDBytes)
		return []uint64{rowID}, nil
	}
	for _, index := range table.Indexes {
		if col == index {
			rowIDBytes, err := s.Get(getIndexKey(table.Name, col, inputField.Data()))
			if err == nil {
				return nil, err
			}
			rowIDs, err := bytesToUint64List(rowIDBytes)
			if err != nil {
				return nil, err
			}
			return rowIDs, nil
		}
	}
	return nil, errors.New("not found in table")
}

func (s *LevelDBOperator) Update(table *DposTable, inputFields []*Field, updateFields []*Field) ([]uint64, error) {
	s.NewBatch()
	if err := s.checkUpdateFields(table, updateFields); err != nil {
		return nil, err
	}

	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}

	for _, id := range ids {
		if err := s.updateRow(table, id, updateFields); err != nil {
			return nil, err
		}
	}
	if err := s.BatchCommit(); err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *LevelDBOperator) checkUpdateFields(table *DposTable, updateFields []*Field) error {
	// check updateFields include exist primary key value
	for _, f := range updateFields {
		if table.Column(f.Name) == table.PrimaryKey {
			if _, err := s.Get(getIndexKey(table.Name, table.PrimaryKey, f.Data())); err == nil {
				return err
			}
		}
	}

	return nil
}

func (s *LevelDBOperator) updateRow(table *DposTable, rowID uint64, updateFields []*Field) error {
	oldFields, err := s.getFieldsByRowID(table, rowID)
	if err != nil {
		return err
	}

	if err := s.updateRowData(table, oldFields, updateFields, rowID); err != nil {
		return err
	}

	for _, f := range updateFields {
		col := table.Column(f.Name)
		if col == table.PrimaryKey {
			if err := s.updatePrimaryKeyValue(table, oldFields, rowID, col, f.Data()); err != nil {
				return err
			}
		}

		var isIndexColumn bool
		var oldData []byte
		for _, field := range table.Fields {
			if col != 0 && col == table.Column(field) {
				oldData = f.Data()
				isIndexColumn = true
			}
		}
		if isIndexColumn {
			if err := s.updateIndexKeyValue(table, rowID, col, oldData, f.Data()); err != nil {
				return err
			}
		}
	}

	return nil
}

func (s *LevelDBOperator) updateRowData(table *DposTable, oldFields []*Field, updateFields []*Field, rowID uint64) error {
	// update row data
	newFieldMap := make(map[string]*Field)
	for _, f := range oldFields {
		newFieldMap[f.Name] = f
	}
	for _, f := range updateFields {
		newFieldMap[f.Name] = f
	}

	var newFields []*Field
	for _, v := range newFieldMap {
		newFields = append(newFields, v)
	}

	data, err := table.Data(newFields)
	if err != nil {
		return err
	}
	s.BatchPut(getRowKey(table.Name, rowID), data)
	return nil
}

func (s *LevelDBOperator) updatePrimaryKeyValue(table *DposTable, oldFields []*Field, rowID uint64, column uint64, newData []byte) error {
	var data []byte
	for _, field := range oldFields {
		if table.Column(field.Name) == table.PrimaryKey {
			data = field.Data()
		}
	}
	oldIndexKey := getIndexKey(table.Name, column, data)
	s.BatchDelete(oldIndexKey)
	newIndexKey := getIndexKey(table.Name, column, newData)
	s.BatchPut(newIndexKey, uint64ToBytes(rowID))
	return nil
}

func (s *LevelDBOperator) updateIndexKeyValue(table *DposTable, rowID uint64, column uint64, oldData []byte, newData []byte) error {
	// if exist index column before, need update old record
	oldIndexKey := getIndexKey(table.Name, column, oldData)
	rowIDBytes, err := s.Get(oldIndexKey)
	if err != nil {
		return err
	}
	rowIDs, err := bytesToUint64List(rowIDBytes)
	if err != nil {
		return err
	}
	if len(rowIDs) <= 1 {
		s.BatchDelete(oldIndexKey)
	} else {
		var newRowIDs []uint64
		for _, r := range rowIDs {
			if r != rowID {
				newRowIDs = append(newRowIDs, r)
			}
		}
		rowIDsListBytes, err := uint64ListToBytes(newRowIDs)
		if err != nil {
			return err
		}
		s.BatchPut(oldIndexKey, rowIDsListBytes)
	}

	// update or add new record
	newIndexKey := getIndexKey(table.Name, column, newData)
	newRowIDBytes, err := s.Get(newIndexKey)
	if err == nil {
		rowIDs, err := bytesToUint64List(rowIDBytes)
		if err != nil {
			return err
		}
		rowIDs = append(rowIDs, rowID)
		rowIDsListBytes, err := uint64ListToBytes(rowIDs)
		if err != nil {
			return err
		}
		s.BatchPut(newIndexKey, rowIDsListBytes)
	} else {
		s.BatchPut(newIndexKey, newRowIDBytes)
	}
	return nil
}

func (s *LevelDBOperator) getFieldsByRowID(table *DposTable, rowID uint64) ([]*Field, error) {
	columnsData, err := s.Get(getRowKey(table.Name, rowID))
	if err != nil {
		return nil, fmt.Errorf("not found row id")
	}

	fields, err := table.GetFields(columnsData)
	if err != nil {
		return nil, err
	}

	return fields, nil
}
