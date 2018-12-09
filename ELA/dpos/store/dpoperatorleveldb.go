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
	db *blockchain.LevelDB
}

func (s *LevelDBOperator) InitConnection(connParams ...interface{}) error {
	return nil
}

func (s *LevelDBOperator) Connect() error {
	return nil
}

func (s *LevelDBOperator) Disconnect() error {
	return nil
}

func (s *LevelDBOperator) Create(table *DposTable) error {
	buf := new(bytes.Buffer)
	if err := table.Serialize(buf); err != nil {
		return err
	}

	key := getTableKey(table.Name)
	_, err := s.db.Get(key)
	if err != nil {
		return fmt.Errorf("already exist table: %s", table.Name)
	}

	if err := s.db.Put(getTableIDKey(table.Name), uint64ToBytes(uint64(0))); err != nil {
		return err
	}

	return s.db.Put(key, buf.Bytes())
}

func (s *LevelDBOperator) Insert(table *DposTable, fields []*Field) (uint64, error) {
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
	idBytes, err := s.db.Get(getTableIDKey(table.Name))
	if err != nil {
		return 0, err
	}
	id := bytesToUint64(idBytes)
	row := id + 1
	data, err := table.Data(fields)
	if err != nil {
		return 0, err
	}
	if err := s.db.Put(getRowKey(tableName, row), data); err != nil {
		return 0, err
	}

	for _, f := range fields {
		col := table.Column(f.Name)
		if col == table.PrimaryKey {
			buf := new(bytes.Buffer)
			common.WriteUint64(buf, id+uint64(1))
			key := getIndexKey(tableName, table.PrimaryKey, f.Data())
			if _, err := s.db.Get(key); err != nil {
				return 0, errors.New("duplicated primary")
			}
			// key: tableName_PrimaryKey_ColumnValue
			// value: rowID
			if err := s.db.Put(key, buf.Bytes()); err != nil {
				return 0, err
			}
		}
		for _, index := range table.Indexes {
			if col == index {
				key := getIndexKey(tableName, col, f.Data())
				var indexes []uint64
				indexData, err := s.db.Get(key)
				if err == nil {
					indexes = bytesToUint64List(indexData)
				}
				indexes = append(indexes, id+uint64(1))
				// key: tableName_IndexID_ColumnValue
				// value: [rowID1,rowID2,rowID3,...]
				if err := s.db.Put(key, uint64ListToBytes(indexes)); err != nil {
					return 0, err
				}
			}
		}
	}

	// update id
	if err := s.db.Put(getTableIDKey(table.Name), uint64ToBytes(id+1)); err != nil {
		return 0, err
	}
	return id + 1, nil
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
		columnsData, err := s.db.Get(getRowKey(table.Name, rowID))
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

// only sport select from primary key or index column
func (s *LevelDBOperator) selectRowsByField(table *DposTable, inputField *Field) ([]uint64, error) {
	col := table.Column(inputField.Name)
	if col == table.PrimaryKey {
		rowIDBytes, err := s.db.Get(getIndexKey(table.Name, col, inputField.Data()))
		if err == nil {
			return nil, err
		}
		rowID := bytesToUint64(rowIDBytes)
		return []uint64{rowID}, nil
	}
	for _, index := range table.Indexes {
		if col == index {
			rowIDBytes, err := s.db.Get(getIndexKey(table.Name, col, inputField.Data()))
			if err == nil {
				return nil, err
			}
			rowIDs := bytesToUint64List(rowIDBytes)
			return rowIDs, nil
		}
	}
	return nil, errors.New("not found in table")
}

func (s *LevelDBOperator) Update(table *DposTable, inputFields []*Field, updateFields []*Field) ([]uint64, error) {
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
	return ids, nil
}

func (s *LevelDBOperator) checkUpdateFields(table *DposTable, updateFields []*Field) error {
	// check updateFields include exist primary key value
	for _, f := range updateFields {
		if table.Column(f.Name) == table.PrimaryKey {
			if _, err := s.db.Get(getIndexKey(table.Name, table.PrimaryKey, f.Data())); err == nil {
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
			if col == table.Column(field) {
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
	if err := s.db.Put(getRowKey(table.Name, rowID), data); err != nil {
		return err
	}
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
	if err := s.db.Delete(oldIndexKey); err != nil {
		return err
	}
	newIndexKey := getIndexKey(table.Name, column, newData)
	if err := s.db.Put(newIndexKey, uint64ToBytes(rowID)); err != nil {
		return err
	}
	return nil
}

func (s *LevelDBOperator) updateIndexKeyValue(table *DposTable, rowID uint64, column uint64, oldData []byte, newData []byte) error {
	// if exist index column before, need update old record
	oldIndexKey := getIndexKey(table.Name, column, oldData)
	rowIDBytes, err := s.db.Get(oldIndexKey)
	if err != nil {
		return err
	}
	rowIDs := bytesToUint64List(rowIDBytes)
	if len(rowIDs) <= 1 {
		if err := s.db.Delete(oldIndexKey); err != nil {
			return err
		}
	} else {
		var newRowIDs []uint64
		for _, r := range rowIDs {
			if r != rowID {
				newRowIDs = append(newRowIDs, r)
			}
		}
		if err := s.db.Put(oldIndexKey, uint64ListToBytes(newRowIDs)); err != nil {
			return err
		}
	}

	// update or add new record
	newIndexKey := getIndexKey(table.Name, column, newData)
	newRowIDBytes, err := s.db.Get(newIndexKey)
	if err == nil {
		rowIDs := bytesToUint64List(rowIDBytes)
		rowIDs = append(rowIDs, rowID)
		if err := s.db.Put(newIndexKey, uint64ListToBytes(rowIDs)); err != nil {
			return err
		}
	} else {
		if err := s.db.Put(newIndexKey, newRowIDBytes); err != nil {
			return err
		}
	}
	return nil
}

func (s *LevelDBOperator) getFieldsByRowID(table *DposTable, rowID uint64) ([]*Field, error) {
	columnsData, err := s.db.Get(getRowKey(table.Name, rowID))
	if err != nil {
		return nil, fmt.Errorf("not found row id")
	}

	fields, err := table.GetFields(columnsData)
	if err != nil {
		return nil, err
	}

	return fields, nil
}
