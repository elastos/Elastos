// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package calculator

import "errors"

const (
	metaFactor = 1000000
)

type sizeCalculator interface {
	initialSize() uint64
	increase() uint64
}

func Calculate(megaSize, format uint) (uint64, error) {
	calculator, err := createCalculator(format)
	if err != nil {
		return 0, err
	}

	maxSize := uint64(megaSize) * metaFactor
	sum := calculator.initialSize()
	count := uint64(0)
	for {
		s := calculator.increase()
		if sum+s > maxSize {
			break
		}
		sum += s
		count++
	}
	return count, nil
}

func createCalculator(format uint) (sizeCalculator, error) {
	switch format {
	case 1:
		return newSingleInputOutput()
	case 2:
		return newMultiInputsOfOneAccount()
	}
	return nil, errors.New("unknown tx format")
}
