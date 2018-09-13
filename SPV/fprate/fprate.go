package fprate

import "github.com/elastos/Elastos.ELA.SPV/util"

const (
	DefaultFalsePositiveRate float64 = 0.0005
	ReducedFalsePositiveRate float64 = 0.00005
)

type FpRate struct {
	averageTxPerBlock float64
	fpRate            float64
}

func (r *FpRate) Update(block *util.Block, fps uint32) (fpRate float64) {
	// moving average number of tx-per-block
	r.averageTxPerBlock = r.averageTxPerBlock*0.999 + float64(block.NumTxs)*0.001

	// 1% low pass filter, also weights each block by total transactions, compared to the avarage
	r.fpRate = r.fpRate*(1.0-0.01*float64(block.NumTxs)/r.averageTxPerBlock) +
		0.01*float64(fps)/r.averageTxPerBlock

	return r.fpRate
}

func (r *FpRate) Reset() {
	r.fpRate = ReducedFalsePositiveRate
}

func NewFpRate() *FpRate {
	return &FpRate{
		averageTxPerBlock: 1400,
		fpRate:            ReducedFalsePositiveRate,
	}
}
