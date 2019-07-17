// Copyright (c) 2015-2016 The btcsuite developers
// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package dtime

import (
	"math"
	"sort"
	"sync"
	"time"
)

const (
	// maxAllowedOffset is the maximum number of nano seconds in either
	// direction that local clock will be adjusted.  When the median time
	// of the network is outside of this range, no offset will be applied.
	maxAllowedOffset = 4.2 * float64(Second) // 4.2 seconds.

	// similarTime is the number of nano seconds in either direction from the
	// local clock that is used to determine that it is likley wrong and
	// hence to show a warning.
	similarTime = 2.1 * float64(Second) // 2.1 seconds.
)

var (
	// maxMedianTimeEntries is the maximum number of entries allowed in the
	// median time data.  This is a variable as opposed to a constant so the
	// test code can modify it.
	maxMedianTimeEntries = 99
)

// MedianTimeSource provides a mechanism to add several time samples which are
// used to determine a median time which is then used as an offset to the local
// clock.
type MedianTimeSource interface {
	// AdjustedTime returns the current time adjusted by the median time
	// offset as calculated from the time samples added by AddTimeSample.
	AdjustedTime() time.Time

	// AddTimeSample adds a time sample that is used when determining the
	// median time of the added samples.
	AddTimeSample(id string, timeVal time.Time)

	// Offset returns the number of nano seconds to adjust the local clock
	// based upon the median of the time samples added by AddTimeData.
	Offset() time.Duration
}

// int64Sorter implements sort.Interface to allow a slice of 64-bit integers to
// be sorted.
type int64Sorter []time.Duration

// Len returns the number of 64-bit integers in the slice.  It is part of the
// sort.Interface implementation.
func (s int64Sorter) Len() int {
	return len(s)
}

// Swap swaps the 64-bit integers at the passed indices.  It is part of the
// sort.Interface implementation.
func (s int64Sorter) Swap(i, j int) {
	s[i], s[j] = s[j], s[i]
}

// Less returns whether the 64-bit integer with index i should sort before the
// 64-bit integer with index j.  It is part of the sort.Interface
// implementation.
func (s int64Sorter) Less(i, j int) bool {
	return s[i] < s[j]
}

// medianTime provides an implementation of the MedianTimeSource interface.
// It is limited to maxMedianTimeEntries includes the same buggy behavior as
// the time offset mechanism in Bitcoin Core.  This is necessary because it is
// used in the consensus code.
type medianTime struct {
	mtx                sync.Mutex
	knownIDs           map[string]struct{}
	offsets            []time.Duration
	offset             time.Duration
	invalidTimeChecked bool
}

// Ensure the medianTime type implements the MedianTimeSource interface.
var _ MedianTimeSource = (*medianTime)(nil)

// AdjustedTime returns the current time adjusted by the median time offset as
// calculated from the time samples added by AddTimeSample.
//
// This function is safe for concurrent access and is part of the
// MedianTimeSource interface implementation.
func (m *medianTime) AdjustedTime() time.Time {
	m.mtx.Lock()
	defer m.mtx.Unlock()

	return Now().Add(m.offset)
}

// AddTimeSample adds a time sample that is used when determining the median
// time of the added samples.
//
// This function is safe for concurrent access and is part of the
// MedianTimeSource interface implementation.
func (m *medianTime) AddTimeSample(sourceID string, timeVal time.Time) {
	m.mtx.Lock()
	defer m.mtx.Unlock()

	// Don't add time data from the same source.
	if _, exists := m.knownIDs[sourceID]; exists {
		return
	}
	m.knownIDs[sourceID] = struct{}{}

	// Truncate the provided offset to million seconds and append it to the
	// slice of offsets while respecting the maximum number of allowed entries
	// by replacing the oldest entry with the new entry once the maximum number
	// of entries is reached.
	offset := timeVal.Sub(Now())
	numOffsets := len(m.offsets)
	if numOffsets == maxMedianTimeEntries && maxMedianTimeEntries > 0 {
		m.offsets = m.offsets[1:]
		numOffsets--
	}
	m.offsets = append(m.offsets, offset)
	numOffsets++

	// Sort the offsets so the median can be obtained as needed later.
	sortedOffsets := make([]time.Duration, numOffsets)
	copy(sortedOffsets, m.offsets)
	sort.Sort(int64Sorter(sortedOffsets))

	log.Debugf("Added time sample of %v (total: %v)", offset.Seconds(),
		numOffsets)

	// The median offset is only updated when there are enough offsets and
	// the number of offsets is odd so the middle value is the true median.
	// Thus, there is nothing to do when those conditions are not met.
	if numOffsets < 5 || numOffsets&0x01 != 1 {
		return
	}

	// At this point the number of offsets in the list is odd, so the
	// middle value of the sorted offsets is the median.
	median := sortedOffsets[numOffsets/2]

	// Set the new offset when the median offset is within the allowed
	// offset range.
	if math.Abs(float64(median)) < maxAllowedOffset {
		m.offset = median
	} else {
		// The median offset of all added time data is larger than the
		// maximum allowed offset, so don't use an offset.  This
		// effectively limits how far the local clock can be skewed.
		m.offset = 0

		if !m.invalidTimeChecked {
			m.invalidTimeChecked = true

			// Find if any time samples have a time that is close
			// to the local time.
			var remoteHasCloseTime bool
			for _, offset := range sortedOffsets {
				if math.Abs(float64(offset)) < similarTime {
					remoteHasCloseTime = true
					break
				}
			}

			// Warn if none of the time samples are close.
			if !remoteHasCloseTime {
				log.Warnf("Please check your date and time " +
					"are correct!  ela will not work " +
					"properly with an invalid time")
			}
		}
	}

	log.Debugf("New time offset: %v", m.offset.Seconds())
}

// Offset returns the number of nano seconds to adjust the local clock based
// upon the median of the time samples added by AddTimeData.
//
// This function is safe for concurrent access and is part of the
// MedianTimeSource interface implementation.
func (m *medianTime) Offset() time.Duration {
	m.mtx.Lock()
	defer m.mtx.Unlock()

	return m.offset
}

// NewMedianTime returns a new instance of concurrency-safe implementation of
// the MedianTimeSource interface.  The returned implementation contains the
// rules necessary for proper time handling in the chain consensus rules and
// expects the time samples to be added from the timestamp field of the version
// message received from remote peers that successfully connect and negotiate.
func NewMedianTime() MedianTimeSource {
	return &medianTime{
		knownIDs: make(map[string]struct{}),
		offsets:  make([]time.Duration, 0, maxMedianTimeEntries),
	}
}
