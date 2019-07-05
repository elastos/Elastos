package bloom

import (
	"bytes"
	"math"
	"sync"

	"github.com/elastos/Elastos.ELA.SPV/util"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

const (
	// MaxFilterLoadHashFuncs is the maximum number of hash functions to
	// load into the Bloom filter.
	MaxFilterLoadHashFuncs = 50

	// MaxFilterLoadFilterSize is the maximum size in bytes a filter may be.
	MaxFilterLoadFilterSize = 36000
)

// ln2Squared is simply the square of the natural log of 2.
const ln2Squared = math.Ln2 * math.Ln2

// minUint32 is a convenience function to return the minimum value of the two
// passed uint32 values.
func minUint32(a, b uint32) uint32 {
	if a < b {
		return a
	}
	return b
}

// Filter defines a bitcoin bloom filter that provides easy manipulation of raw
// filter data.
type Filter struct {
	mtx sync.Mutex
	msg *msg.FilterLoad
}

// NewFilter creates a new bloom filter instance, mainly to be used by SPV
// clients.  The tweak parameter is a random value added to the seed value.
// The false positive rate is the probability of a false positive where 1.0 is
// "match everything" and zero is unachievable.  Thus, providing any false
// positive rates less than 0 or greater than 1 will be adjusted to the valid
// range.
//
// For more information on what values to use for both elements and fprate,
// see https://en.wikipedia.org/wiki/Bloom_filter.
func NewFilter(elements, tweak uint32, fprate float64) *Filter {
	// Massage the false positive rate to sane values.
	if fprate > 1.0 {
		fprate = 1.0
	}
	if fprate < 1e-9 {
		fprate = 1e-9
	}

	// Calculate the size of the filter in bytes for the given number of
	// elements and false positive rate.
	//
	// Equivalent to m = -(n*ln(p) / ln(2)^2), where m is in bits.
	// Then clamp it to the maximum filter size and convert to bytes.
	dataLen := uint32(-1 * float64(elements) * math.Log(fprate) / ln2Squared)
	dataLen = minUint32(dataLen, MaxFilterLoadFilterSize*8) / 8

	// Calculate the number of hash functions based on the size of the
	// filter calculated above and the number of elements.
	//
	// Equivalent to k = (m/n) * ln(2)
	// Then clamp it to the maximum allowed hash funcs.
	hashFuncs := uint32(float64(dataLen*8) / float64(elements) * math.Ln2)
	hashFuncs = minUint32(hashFuncs, MaxFilterLoadHashFuncs)

	msg := &msg.FilterLoad{
		Filter:    make([]byte, dataLen),
		HashFuncs: hashFuncs,
		Tweak:     tweak,
	}
	return &Filter{msg: msg}
}

// LoadFilter creates a new Filter instance with the given underlying
// msg.FilterLoad.
func LoadFilter(msg *msg.FilterLoad) *Filter {
	filter := new(Filter)
	filter.msg = msg
	return filter
}

// IsLoaded returns true if a filter is loaded, otherwise false.
//
// This function is safe for concurrent access.
func (bf *Filter) IsLoaded() bool {
	bf.mtx.Lock()
	loaded := bf.msg != nil
	bf.mtx.Unlock()
	return loaded
}

// Reload loads a new filter replacing any existing filter.
//
// This function is safe for concurrent access.
func (bf *Filter) Reload(msg *msg.FilterLoad) {
	bf.mtx.Lock()
	bf.msg = msg
	bf.mtx.Unlock()
}

// Unload unloads the bloom filter.
//
// This function is safe for concurrent access.
func (bf *Filter) Unload() {
	bf.mtx.Lock()
	bf.msg = nil
	bf.mtx.Unlock()
}

// hash returns the bit offset in the bloom filter which corresponds to the
// passed data for the given indepedent hash function number.
func (bf *Filter) hash(hashNum uint32, data []byte) uint32 {
	// bitcoind: 0xfba4c795 chosen as it guarantees a reasonable bit
	// difference between hashNum values.
	//
	// Note that << 3 is equivalent to multiplying by 8, but is faster.
	// Thus the returned hash is brought into range of the number of bits
	// the filter has and returned.
	mm := MurmurHash3(hashNum*0xfba4c795+bf.msg.Tweak, data)
	return mm % (uint32(len(bf.msg.Filter)) << 3)
}

// matches returns true if the bloom filter might contain the passed data and
// false if it definitely does not.
//
// This function MUST be called with the filter lock held.
func (bf *Filter) matches(data []byte) bool {
	if bf.msg == nil {
		return false
	}

	// The bloom filter does not contain the data if any of the bit offsets
	// which result from hashing the data using each independent hash
	// function are not set.  The shifts and masks below are a faster
	// equivalent of:
	//   arrayIndex := idx / 8     (idx >> 3)
	//   bitOffset := idx % 8      (idx & 7)
	///  if filter[arrayIndex] & 1<<bitOffset == 0 { ... }
	for i := uint32(0); i < bf.msg.HashFuncs; i++ {
		idx := bf.hash(i, data)
		if bf.msg.Filter[idx>>3]&(1<<(idx&7)) == 0 {
			return false
		}
	}
	return true
}

// Matches returns true if the bloom filter might contain the passed data and
// false if it definitely does not.
//
// This function is safe for concurrent access.
func (bf *Filter) Matches(data []byte) bool {
	bf.mtx.Lock()
	match := bf.matches(data)
	bf.mtx.Unlock()
	return match
}

// matchesOutPoint returns true if the bloom filter might contain the passed
// outpoint and false if it definitely does not.
//
// This function MUST be called with the filter lock held.
func (bf *Filter) matchesOutPoint(outpoint *util.OutPoint) bool {
	return bf.matches(outpoint.Bytes())
}

// MatchesOutPoint returns true if the bloom filter might contain the passed
// outpoint and false if it definitely does not.
//
// This function is safe for concurrent access.
func (bf *Filter) MatchesOutPoint(outpoint *util.OutPoint) bool {
	bf.mtx.Lock()
	match := bf.matchesOutPoint(outpoint)
	bf.mtx.Unlock()
	return match
}

// add adds the passed byte slice to the bloom filter.
//
// This function MUST be called with the filter lock held.
func (bf *Filter) add(data []byte) {
	if bf.msg == nil {
		return
	}

	// Adding data to a bloom filter consists of setting all of the bit
	// offsets which result from hashing the data using each independent
	// hash function.  The shifts and masks below are a faster equivalent
	// of:
	//   arrayIndex := idx / 8    (idx >> 3)
	//   bitOffset := idx % 8     (idx & 7)
	///  filter[arrayIndex] |= 1<<bitOffset
	for i := uint32(0); i < bf.msg.HashFuncs; i++ {
		idx := bf.hash(i, data)
		bf.msg.Filter[idx>>3] |= (1 << (7 & idx))
	}
}

// Add adds the passed byte slice to the bloom filter.
//
// This function is safe for concurrent access.
func (bf *Filter) Add(data []byte) {
	bf.mtx.Lock()
	bf.add(data)
	bf.mtx.Unlock()
}

// AddHash adds the passed chainhash.Hash to the Filter.
//
// This function is safe for concurrent access.
func (bf *Filter) AddHash(hash *common.Uint256) {
	bf.mtx.Lock()
	bf.add(hash[:])
	bf.mtx.Unlock()
}

// addOutPoint adds the passed tx outpoint to the bloom filter.
//
// This function MUST be called with the filter lock held.
func (bf *Filter) addOutPoint(outpoint *util.OutPoint) {
	bf.add(outpoint.Bytes())
}

// AddOutPoint adds the passed tx outpoint to the bloom filter.
//
// This function is safe for concurrent access.
func (bf *Filter) AddOutPoint(outpoint *util.OutPoint) {
	bf.mtx.Lock()
	bf.addOutPoint(outpoint)
	bf.mtx.Unlock()
}

func (bf *Filter) GetFilterLoadMsg() *msg.FilterLoad {
	return bf.msg
}

// ToTxFilterMsg parse the bloom filter into a *msg.TxFilterLoad instance.
func (bf *Filter) ToTxFilterMsg(typ uint8) *msg.TxFilterLoad {
	buf := new(bytes.Buffer)
	bf.msg.Serialize(buf)
	return &msg.TxFilterLoad{Type: typ, Data: buf.Bytes()}
}
