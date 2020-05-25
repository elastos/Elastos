// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package gpath

import (
	"errors"
	"fmt"
	"reflect"
	"strings"
)

func IsNil(value interface{}) bool {
	valueType := reflect.TypeOf(value).Kind()
	switch valueType {
	case reflect.Ptr, reflect.Array, reflect.Chan, reflect.Map, reflect.Slice:
		return reflect.DeepEqual(value,
			reflect.Zero(reflect.TypeOf(value)).Interface())
	default:
		return false
	}
}

func Equal(a, b interface{}) bool {
	return reflect.DeepEqual(a, b)
}

func At(str interface{}, path string) (interface{}, error) {
	fields := strings.Split(path, ".")
	value, err := at(direct(reflect.ValueOf(str)), fields)
	if err != nil {
		return nil, err
	}

	if value == (reflect.Value{}) {
		return nil, nil
	}

	return value.Interface(), nil
}

func Set(src, targetValue interface{}, path string) error {
	fields := strings.Split(path, ".")

	srcValue := reflect.ValueOf(src).Elem()
	if srcValue.Kind() != reflect.Struct {
		return errors.New("src should be pointer of struct")
	}
	return set(srcValue, reflect.ValueOf(targetValue), fields)
}

func at(src reflect.Value, fields []string) (reflect.Value, error) {
	if len(fields) == 0 {
		return src, nil
	} else {
		v := src.FieldByName(fields[0])
		if !v.IsValid() {
			return reflect.Value{}, fmt.Errorf("invalid filed name %s",
				fields[0])
		}
		subFields := fields[1:]
		return at(v, subFields)
	}
}

func set(src, tar reflect.Value, fields []string) error {
	if len(fields) == 0 {
		if !src.CanSet() {
			return fmt.Errorf("filed %s is readonly", fields[0])
		}
		if src.Type() != tar.Type() {
			return errors.New("target value type not matched")
		}
		src.Set(tar)
		return nil
	} else {
		v := src.FieldByName(fields[0])
		if !v.IsValid() {
			return fmt.Errorf("invalid filed name %s", fields[0])
		}
		subFields := fields[1:]
		return set(v, tar, subFields)
	}
}

func direct(v reflect.Value) reflect.Value {
	switch v.Kind() {
	case reflect.Ptr, reflect.Interface:
		return v.Elem()
	default:
		return v
	}
}
