package servers

import (
	"strconv"
)

type Params map[string]interface{}

func FromArray(array []interface{}, fileds ...string) Params {
	params := make(Params)
	for i := 0; i < len(array); i++ {
		params[fileds[i]] = array[i]
	}
	return params
}

func (p Params) Int(filed string) (int64, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}
	switch v := value.(type) {
	case float64:
		return int64(v), true
	case string:
		int, err := strconv.ParseInt(p[filed].(string), 10, 64)
		if err != nil {
			return 0, false
		}
		return int, true
	default:
		return 0, false
	}
}

func (p Params) Uint(filed string) (uint32, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}
	switch v := value.(type) {
	case float64:
		if v < 0 {
			return 0, false
		}
		return uint32(v), true
	case string:
		uint, err := strconv.ParseUint(p[filed].(string), 10, 64)
		if err != nil {
			return 0, false
		}
		return uint32(uint), true
	default:
		return 0, false
	}
}

func (p Params) Float(filed string) (float64, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}
	switch v := value.(type) {
	case float64:
		return v, true
	case string:
		float, err := strconv.ParseFloat(p[filed].(string), 64)
		if err != nil {
			return 0, false
		}

		return float, true
	default:
		return 0, false
	}
}

func (p Params) Bool(key string) (bool, bool) {
	value, ok := p[key]
	if !ok {
		return false, false
	}
	switch v := value.(type) {
	case bool:
		return v, true
	default:
		return false, false
	}
}

func (p Params) String(key string) (string, bool) {
	value, ok := p[key]
	if !ok {
		return "", false
	}
	switch v := value.(type) {
	case string:
		return v, true
	default:
		return "", false
	}
}
