package store

import (
	"bytes"
	"errors"
	"fmt"
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common"
)

type persistTask interface{}

type DposStore struct {
	Database

	taskCh chan persistTask

	wg   sync.WaitGroup
	quit chan struct{}
}

func NewDposStore(filePath string) (blockchain.IDposStore, error) {
	store := &DposStore{}
	if err := store.InitConnection(filePath); err != nil {
		return nil, err
	}

	store.StartRecordEvent()
	store.StartRecordArbitrators()

	return store, nil
}

func (s *DposStore) InitConnection(connParams ...interface{}) error {
	filePath, ok := connParams[0].(string)
	if !ok {
		return errors.New("[InitConnection] Invalid sql db file path.")
	}

	db, err := NewLevelDB(filePath)
	if err != nil {
		return errors.New("[InitConnection] database connect failed.")
	}

	s.Database = db
	s.taskCh = make(chan persistTask, MaxEvnetTaskNumber)
	s.quit = make(chan struct{}, 1)

	return nil
}

func (s *DposStore) Disconnect() error {
	close(s.quit)
	s.wg.Wait()
	s.Close()
	return nil
}

func (s *DposStore) Create(table *blockchain.DBTable) error {
	buf := new(bytes.Buffer)
	if err := table.Serialize(buf); err != nil {
		return err
	}

	key := blockchain.GetTableKey(table.Name)
	_, err := s.Get(key)
	if err == nil {
		return fmt.Errorf("already exist table: %s", table.Name)
	}

	if err := s.Put(blockchain.GetTableIDKey(table.Name), blockchain.Uint64ToBytes(uint64(0))); err != nil {
		return err
	}

	return s.Put(key, buf.Bytes())
}

func (s *DposStore) Insert(table *blockchain.DBTable, fields []*blockchain.Field) (uint64, error) {
	batch := s.NewBatch()
	tableName := table.Name

	// key: tableName_rowID
	// value: [colvalue1, colvalue2, colvalue3, ...]
	idBytes, err := s.Get(blockchain.GetTableIDKey(table.Name))
	if err != nil {
		return 0, err
	}
	id := blockchain.BytesToUint64(idBytes)
	rowID := id + 1
	data, err := table.Data(fields)
	if err != nil {
		return 0, err
	}
	batch.Put(blockchain.GetRowKey(tableName, rowID), data)

	for _, f := range fields {
		col := table.Column(f.Name)
		if col == table.PrimaryKey {
			buf := new(bytes.Buffer)
			common.WriteUint64(buf, rowID)
			key := blockchain.GetIndexKey(tableName, table.PrimaryKey, f.Data())
			if _, err := s.Get(key); err == nil {
				return 0, errors.New("duplicated primary")
			}
			// key: tableName_PrimaryKey_ColumnValue
			// value: rowID
			batch.Put(key, buf.Bytes())
		}
		for _, index := range table.Indexes {
			if col == index {
				key := blockchain.GetIndexKey(tableName, col, f.Data())
				var indexes []uint64
				indexData, err := s.Get(key)
				if err == nil {
					indexes, err = blockchain.BytesToUint64List(indexData)
					if err != nil {
						return 0, err
					}
				}
				indexes = append(indexes, rowID)
				// key: tableName_IndexID_ColumnValue
				// value: [rowID1,rowID2,rowID3,...]
				indexListBytes, err := blockchain.Uint64ListToBytes(indexes)
				if err != nil {
					return 0, err
				}
				batch.Put(key, indexListBytes)
			}
		}
	}

	// update id
	batch.Put(blockchain.GetTableIDKey(table.Name), blockchain.Uint64ToBytes(rowID))
	if err := batch.Commit(); err != nil {
		return 0, err
	}
	return rowID, nil
}

func (s *DposStore) Select(table *blockchain.DBTable, inputFields []*blockchain.Field) ([][]*blockchain.Field, error) {
	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}

	return s.selectValuesFromRowIDs(table, ids)
}

func (s *DposStore) SelectID(table *blockchain.DBTable, inputFields []*blockchain.Field) ([]uint64, error) {
	ids, err := s.selectRowIDs(table, inputFields)
	if err != nil {
		return nil, err
	}
	return ids, nil
}

func (s *DposStore) selectValuesFromRowIDs(table *blockchain.DBTable, rowIDs []uint64) ([][]*blockchain.Field, error) {
	var result [][]*blockchain.Field
	for _, rowID := range rowIDs {
		columnsData, err := s.Get(blockchain.GetRowKey(table.Name, rowID))
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

func (s *DposStore) selectRowIDs(table *blockchain.DBTable, inputFields []*blockchain.Field) ([]uint64, error) {
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
func (s *DposStore) selectRowsByField(table *blockchain.DBTable, inputField *blockchain.Field) ([]uint64, error) {
	col := table.Column(inputField.Name)
	if col == table.PrimaryKey {
		rowIDBytes, err := s.Get(blockchain.GetIndexKey(table.Name, col, inputField.Data()))
		if err != nil {
			return nil, err
		}
		rowID := blockchain.BytesToUint64(rowIDBytes)
		return []uint64{rowID}, nil
	}
	for _, index := range table.Indexes {
		if col == index {
			rowIDBytes, err := s.Get(blockchain.GetIndexKey(table.Name, col, inputField.Data()))
			if err != nil {
				return nil, err
			}
			rowIDs, err := blockchain.BytesToUint64List(rowIDBytes)
			if err != nil {
				return nil, err
			}
			return rowIDs, nil
		}
	}
	return nil, errors.New("not found in table")
}

func (s *DposStore) Update(table *blockchain.DBTable, inputFields []*blockchain.Field, updateFields []*blockchain.Field) ([]uint64, error) {
	batch := s.NewBatch()
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

func (s *DposStore) checkUpdateFields(table *blockchain.DBTable, updateFields []*blockchain.Field) error {
	// check updateFields include exist primary key value
	for _, f := range updateFields {
		if table.Column(f.Name) == table.PrimaryKey {
			if _, err := s.Get(blockchain.GetIndexKey(table.Name, table.PrimaryKey, f.Data())); err == nil {
				return err
			}
		}
	}

	return nil
}

func (s *DposStore) updateRow(batch Batch, table *blockchain.DBTable, rowID uint64, updateFields []*blockchain.Field) error {
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

func (s *DposStore) updateRowData(batch Batch, table *blockchain.DBTable, oldFields []*blockchain.Field, updateFields []*blockchain.Field, rowID uint64) error {
	// update row data
	newFieldMap := make(map[string]*blockchain.Field)
	for _, f := range oldFields {
		newFieldMap[f.Name] = f
	}
	for _, f := range updateFields {
		newFieldMap[f.Name] = f
	}

	var newFields []*blockchain.Field
	for _, v := range newFieldMap {
		newFields = append(newFields, v)
	}

	data, err := table.Data(newFields)
	if err != nil {
		return err
	}
	batch.Put(blockchain.GetRowKey(table.Name, rowID), data)
	return nil
}

func (s *DposStore) updatePrimaryKeyValue(batch Batch, table *blockchain.DBTable, oldFields []*blockchain.Field, rowID uint64, column uint64, newData []byte) error {
	var oldData []byte
	for _, field := range oldFields {
		if table.Column(field.Name) == table.PrimaryKey {
			oldData = field.Data()
		}
	}
	oldIndexKey := blockchain.GetIndexKey(table.Name, column, oldData)
	batch.Delete(oldIndexKey)
	newIndexKey := blockchain.GetIndexKey(table.Name, column, newData)
	batch.Put(newIndexKey, blockchain.Uint64ToBytes(rowID))
	return nil
}

func (s *DposStore) updateIndexKeyValue(batch Batch, table *blockchain.DBTable, rowID uint64, column uint64, oldData []byte, newData []byte) error {

	// if exist index column before, need update old record
	oldIndexKey := blockchain.GetIndexKey(table.Name, column, oldData)
	rowIDBytes, err := s.Get(oldIndexKey)
	if err != nil {
		return err
	}
	rowIDs, err := blockchain.BytesToUint64List(rowIDBytes)
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
		rowIDsListBytes, err := blockchain.Uint64ListToBytes(newRowIDs)
		if err != nil {
			return err
		}
		batch.Put(oldIndexKey, rowIDsListBytes)
	}

	// update or add new record
	newIndexKey := blockchain.GetIndexKey(table.Name, column, newData)
	newRowIDBytes, err := s.Get(newIndexKey)
	if err == nil {
		rowIDs, err := blockchain.BytesToUint64List(rowIDBytes)
		if err != nil {
			return err
		}
		rowIDs = append(rowIDs, rowID)
		rowIDsListBytes, err := blockchain.Uint64ListToBytes(rowIDs)
		if err != nil {
			return err
		}
		batch.Put(newIndexKey, rowIDsListBytes)
	} else {
		batch.Put(newIndexKey, newRowIDBytes)
	}
	return nil
}

func (s *DposStore) getFieldsByRowID(table *blockchain.DBTable, rowID uint64) ([]*blockchain.Field, error) {
	columnsData, err := s.Get(blockchain.GetRowKey(table.Name, rowID))
	if err != nil {
		return nil, fmt.Errorf("not found row id")
	}

	fields, err := table.GetFields(columnsData)
	if err != nil {
		return nil, err
	}

	return fields, nil
}

func (s *DposStore) deleteTable(table *blockchain.DBTable) error {
	buf := new(bytes.Buffer)
	if err := table.Serialize(buf); err != nil {
		return err
	}

	key := blockchain.GetTableKey(table.Name)
	_, err := s.Get(key)
	if err != nil {
		return fmt.Errorf("not exist table: %s", table.Name)
	}
	rowCount, err := s.Get(blockchain.GetTableIDKey(table.Name))
	if err != nil {
		return err
	}
	var rowIDs []uint64
	for i := uint64(0); i < blockchain.BytesToUint64(rowCount); i++ {
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

	for rowID, fs := range fields {
		for _, f := range fs {
			// delete primary value
			if table.Column(f.Name) == table.PrimaryKey {
				s.Delete(blockchain.GetIndexKey(table.Name, table.PrimaryKey, f.Data()))
			}
			// delete index value
			if _, ok := indexes[table.Column(f.Name)]; ok {
				s.Delete(blockchain.GetIndexKey(table.Name, table.Column(f.Name), f.Data()))
			}
		}
		// delete row data
		s.Delete(blockchain.GetRowKey(table.Name, uint64(rowID)))
	}
	// delete row count
	s.Delete(blockchain.GetTableIDKey(table.Name))
	// delete table key
	s.Delete(blockchain.GetTableKey(table.Name))

	return nil
}
