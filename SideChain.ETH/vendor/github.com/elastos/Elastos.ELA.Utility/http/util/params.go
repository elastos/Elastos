package util

import (
	"strconv"
)

type Params map[string]interface{}

func (p Params) Int(filed string) (int, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		return int(v), true

	case string:
		i, err := strconv.ParseInt(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return int(i), true

	default:
		return 0, false
	}
}

func (p Params) Int16(filed string) (int16, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		return int16(v), true

	case string:
		i, err := strconv.ParseInt(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return int16(i), true

	default:
		return 0, false
	}
}

func (p Params) Int32(filed string) (int32, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		return int32(v), true

	case string:
		i, err := strconv.ParseInt(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return int32(i), true

	default:
		return 0, false
	}
}

func (p Params) Int64(filed string) (int64, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		return int64(v), true

	case string:
		i, err := strconv.ParseInt(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return i, true

	default:
		return 0, false
	}
}

func (p Params) Uint(filed string) (uint, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		if v < 0 {
			return 0, false
		}
		return uint(v), true

	case string:
		ui, err := strconv.ParseUint(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return uint(ui), true

	default:
		return 0, false
	}
}

func (p Params) Uint16(filed string) (uint16, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		if v < 0 {
			return 0, false
		}
		return uint16(v), true

	case string:
		ui, err := strconv.ParseUint(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return uint16(ui), true

	default:
		return 0, false
	}
}

func (p Params) Uint32(filed string) (uint32, bool) {
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
		ui, err := strconv.ParseUint(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return uint32(ui), true

	default:
		return 0, false
	}
}

func (p Params) Uint64(filed string) (uint64, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		if v < 0 {
			return 0, false
		}
		return uint64(v), true

	case string:
		ui, err := strconv.ParseUint(v, 10, 64)
		if err != nil {
			return 0, false
		}
		return ui, true

	default:
		return 0, false
	}
}

func (p Params) Float32(filed string) (float32, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		return float32(v), true

	case string:
		f, err := strconv.ParseFloat(v, 64)
		if err != nil {
			return 0, false
		}
		return float32(f), true

	default:
		return 0, false
	}
}

func (p Params) Float64(filed string) (float64, bool) {
	value, ok := p[filed]
	if !ok {
		return 0, false
	}

	switch v := value.(type) {
	case float64:
		return v, true

	case string:
		f, err := strconv.ParseFloat(v, 64)
		if err != nil {
			return 0, false
		}
		return f, true

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
