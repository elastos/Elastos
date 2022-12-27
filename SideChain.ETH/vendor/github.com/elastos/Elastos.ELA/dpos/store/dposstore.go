package store

import (
	"bytes"
	"errors"
	"fmt"
	"path/filepath"
	"sync"

	"github.com/elastos/Elastos.ELA/common"
)

type eventTask interface{}
type persistTask interface{}

// Ensure DposStore implement the IDposStore interface.
var _ IDposStore = (*DposStore)(nil)

type DposStore struct {
	db Database

	eventCh   chan eventTask
	persistCh chan persistTask

	wg   sync.WaitGroup
	quit chan struct{}
}

func NewDposStore(dataDir string) (*DposStore, error) {
	db, err := NewLevelDB(filepath.Join(dataDir, "dpos"))
	if err != nil {
		return nil, err
	}

	s := DposStore{
		db:        db,
		eventCh:   make(chan eventTask, MaxEvnetTaskNumber),
		persistCh: make(chan persistTask, MaxEvnetTaskNumber),
		quit:      make(chan struct{}),
	}

	return &s, nil
}

func (s *DposStore) Close() error {
	close(s.quit)
	s.wg.Wait()
	s.db.Close()
	return nil
}

func (s *DposStore) Create(table *DBTable) error {
	buf := new(bytes.Buffer)
	if err := table.Serialize(buf); err != nil {
		return err
	}

	key := GetTableKey(table.Name)
	_, err := s.db.Get(key)
	if err == nil {
		return fmt.Errorf("already exist table: %s", table.Name)
	}

	if err := s.db.Put(GetTableIDKey(table.Name), Uint64ToBytes(uint64(0))); err != nil {
		return err
	}

	return s.db.Put(key, buf.Bytes())
}

func (s *DposStore) Insert(table *DBTable, fields []*Field) (uint64, error) {
	batch := s.db.NewBatch()
	tableName := table.Name

	// key: tableName_rowID
	// value: [colvalue1, colvalue2, colvalue3, ...]
	idBytes, err := s.db.Get(GetTableIDKey(table.Name))
	if err != nil {
		return 0, err
	}
	id := BytesToUint64(idBytes)
	rowID := id + 1
	data, err := table.Data(fields)
	if err != nil {
		return 0, err
	}
	batch.Put(GetRowKey(tableName, rowID), data)

	for _, f := range fields {
		col := table.Column(f.Name)
		if col == table.PrimaryKey {
			buf := new(bytes.Buffer)
			common.WriteUint64(buf, rowID)
			key := GetIndexKey(tableName, table.PrimaryKey, f.Data())
			if _, err := s.db.Get(key); err == nil {
				return 0, errors.New("duplicated primary")
			}
			// key: tableName_PrimaryKey_ColumnValue
			// value: rowID
			batch.Put(key, buf.Bytes())
		}
		for _, index := range table.Indexes {
			if col == index {
				key := GetIndexKey(tableName, col, f.Data())
				var indexes []uint64
				indexData, err := s.db.Get(key)
				if err == nil {
					indexes, err = BytesToUint64List(indexData)
					if err != nil {
						return 0, err
					}
				}
				indexes = append(indexes, rowID)
				// key: tableName_IndexID_ColumnValue
				// value: [rowID1,rowID2,rowID3,...]
				indexListBytes, err := Uint64ListToBytes(indexes)
				if err != nil {
					return 0, err
				}
				batch.Put(key, indexListBytes)
			}
		}
	}

	// update id
	batch.Put(GetTableIDKey(table.Name), Uint64ToBytes(rowID))
	if err := batch.Commit(); err != nil {
		return 0, err
	}
	return rowID, nil
}

func (s *DposStore) Select(table *DBTable, inputFields []*Field) ([][]*Field, error) {
	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}

	return s.selectValuesFromRowIDs(table, ids)
}

func (s *DposStore) SelectID(table *DBTable, inputFields []*Field) ([]uint64, error) {
	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *DposStore) selectValuesFromRowIDs(table *DBTable, rowIDs []uint64) ([][]*Field, error) {
	var result [][]*Field
	for _, rowID := range rowIDs {
		columnsData, err := s.db.Get(GetRowKey(table.Name, rowID))
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

func (s *DposStore) selectRowIDs(table *DBTable, inputFields []*Field) ([]uint64, error) {
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
func (s *DposStore) selectRowsByField(table *DBTable, inputField *Field) ([]uint64, error) {
	col := table.Column(inputField.Name)
	if col == table.PrimaryKey {
		rowIDBytes, err := s.db.Get(GetIndexKey(table.Name, col, inputField.Data()))
		if err != nil {
			return nil, err
		}
		rowID := BytesToUint64(rowIDBytes)
		return []uint64{rowID}, nil
	}
	for _, index := range table.Indexes {
		if col == index {
			rowIDBytes, err := s.db.Get(GetIndexKey(table.Name, col, inputField.Data()))
			if err != nil {
				return nil, err
			}
			rowIDs, err := BytesToUint64List(rowIDBytes)
			if err != nil {
				return nil, err
			}
			return rowIDs, nil
		}
	}
	return nil, errors.New("not found in table")
}

func (s *DposStore) Update(table *DBTable, inputFields []*Field, updateFields []*Field) ([]uint64, error) {
	batch := s.db.NewBatch()
	if err := s.checkUpdateFields(table, updateFields); err != nil {
		return nil, err
	}

	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}

	for _, id := range ids {
		if err := s.updateRow(batch, table, id, updateFields); err != nil {
			return nil, err
		}
	}
	if err := batch.Commit(); err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *DposStore) checkUpdateFields(table *DBTable, updateFields []*Field) error {
	// check updateFields include exist primary key value
	for _, f := range updateFields {
		if table.Column(f.Name) == table.PrimaryKey {
			if _, err := s.db.Get(GetIndexKey(table.Name, table.PrimaryKey, f.Data())); err == nil {
				return err
			}
		}
	}

	return nil
}

func (s *DposStore) updateRow(batch Batch, table *DBTable, rowID uint64, updateFields []*Field) error {
	oldFields, err := s.getFieldsByRowID(table, rowID)
	if err != nil {
		return err
	}

	if err := s.updateRowData(batch, table, oldFields, updateFields, rowID); err != nil {
		return err
	}

	indexes := make(map[uint64]struct{})
	for _, index := range table.Indexes {
		indexes[index] = struct{}{}
	}

	for _, f := range updateFields {
		col := table.Column(f.Name)
		if col == table.PrimaryKey {
			if err := s.updatePrimaryKeyValue(batch, table, oldFields, rowID, col, f.Data()); err != nil {
				return err
			}
		}

		if _, ok := indexes[col]; ok {
			var oldData []byte
			for _, field := range oldFields {
				if field.Name == f.Name {
					oldData = field.Data()
				}
			}
			if err := s.updateIndexKeyValue(batch, table, rowID, col, oldData, f.Data()); err != nil {
				return err
			}
		}
	}

	return nil
}

func (s *DposStore) updateRowData(batch Batch, table *DBTable, oldFields []*Field, updateFields []*Field, rowID uint64) error {
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
	batch.Put(GetRowKey(table.Name, rowID), data)
	return nil
}

func (s *DposStore) updatePrimaryKeyValue(batch Batch, table *DBTable, oldFields []*Field, rowID uint64, column uint64, newData []byte) error {
	var oldData []byte
	for _, field := range oldFields {
		if table.Column(field.Name) == table.PrimaryKey {
			oldData = field.Data()
		}
	}
	oldIndexKey := GetIndexKey(table.Name, column, oldData)
	batch.Delete(oldIndexKey)
	newIndexKey := GetIndexKey(table.Name, column, newData)
	batch.Put(newIndexKey, Uint64ToBytes(rowID))
	return nil
}

func (s *DposStore) updateIndexKeyValue(batch Batch, table *DBTable, rowID uint64, column uint64, oldData []byte, newData []byte) error {
	// if exist index column before, need update old record
	oldIndexKey := GetIndexKey(table.Name, column, oldData)
	rowIDBytes, err := s.db.Get(oldIndexKey)
	if err != nil {
		return err
	}
	rowIDs, err := BytesToUint64List(rowIDBytes)
	if err != nil {
		return err
	}
	if len(rowIDs) <= 1 {
		batch.Delete(oldIndexKey)
	} else {
		var newRowIDs []uint64
		for _, r := range rowIDs {
			if r != rowID {
				newRowIDs = append(newRowIDs, r)
			}
		}
		rowIDsListBytes, err := Uint64ListToBytes(newRowIDs)
		if err != nil {
			return err
		}
		batch.Put(oldIndexKey, rowIDsListBytes)
	}

	// update or add new record
	newIndexKey := GetIndexKey(table.Name, column, newData)
	newRowIDBytes, err := s.db.Get(newIndexKey)
	if err == nil {
		rowIDs, err := BytesToUint64List(rowIDBytes)
		if err != nil {
			return err
		}
		rowIDs = append(rowIDs, rowID)
		rowIDsListBytes, err := Uint64ListToBytes(rowIDs)
		if err != nil {
			return err
		}
		batch.Put(newIndexKey, rowIDsListBytes)
	} else {
		batch.Put(newIndexKey, newRowIDBytes)
	}
	return nil
}

func (s *DposStore) getFieldsByRowID(table *DBTable, rowID uint64) ([]*Field, error) {
	columnsData, err := s.db.Get(GetRowKey(table.Name, rowID))
	if err != nil {
		return nil, fmt.Errorf("not found row id")
	}

	fields, err := table.GetFields(columnsData)
	if err != nil {
		return nil, err
	}

	return fields, nil
}

func (s *DposStore) deleteTable(table *DBTable) error {
	buf := new(bytes.Buffer)
	if err := table.Serialize(buf); err != nil {
		return err
	}

	key := GetTableKey(table.Name)
	_, err := s.db.Get(key)
	if err != nil {
		return fmt.Errorf("not exist table: %s", table.Name)
	}
	rowCount, err := s.db.Get(GetTableIDKey(table.Name))
	if err != nil {
		return err
	}
	var rowIDs []uint64
	for i := uint64(0); i < BytesToUint64(rowCount); i++ {
		rowIDs = append(rowIDs, i+1)
	}
	fields, err := s.selectValuesFromRowIDs(table, rowIDs)
	if err != nil {
		return err
	}

	indexes := make(map[uint64]struct{})
	for _, index := range table.Indexes {
		indexes[index] = struct{}{}
	}

	batch := s.db.NewBatch()
	for rowID, fs := range fields {
		for _, f := range fs {
			// delete primary value
			if table.Column(f.Name) == table.PrimaryKey {
				batch.Delete(GetIndexKey(table.Name, table.PrimaryKey, f.Data()))
			}
			// delete index value
			if _, ok := indexes[table.Column(f.Name)]; ok {
				batch.Delete(GetIndexKey(table.Name, table.Column(f.Name), f.Data()))
			}
		}
		// delete row data
		batch.Delete(GetRowKey(table.Name, uint64(rowID)))
	}
	// delete row count
	batch.Delete(GetTableIDKey(table.Name))
	// delete table key
	batch.Delete(GetTableKey(table.Name))

	return batch.Commit()
}
