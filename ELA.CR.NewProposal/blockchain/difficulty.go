package blockchain

import (
	"errors"
	"math/big"
	"time"

	"github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

var (
	targetTimespan      = int64(config.Parameters.ChainParam.TargetTimespan / time.Second)
	targetTimePerBlock  = int64(config.Parameters.ChainParam.TargetTimePerBlock / time.Second)
	blocksPerRetarget   = uint32(targetTimespan / targetTimePerBlock)
	minRetargetTimespan = int64(targetTimespan / config.Parameters.ChainParam.AdjustmentFactor)
	maxRetargetTimespan = int64(targetTimespan * config.Parameters.ChainParam.AdjustmentFactor)
)

func CalcNextRequiredDifficulty(prevNode *BlockNode, newBlockTime time.Time) (uint32, error) {
	// Genesis block.
	if (prevNode.Height == 0) || (config.Parameters.ChainParam.Name == "RegNet") {
		return uint32(config.Parameters.ChainParam.PowLimitBits), nil

	}

	// Return the previous block's difficulty requirements if this block
	// is not at a difficulty retarget interval.
	if (prevNode.Height+1)%blocksPerRetarget != 0 {
		return prevNode.Bits, nil
	}

	// Get the block node at the previous retarget (targetTimespan days
	// worth of blocks).
	height := prevNode.Height - blocksPerRetarget + 1
	if height < 0 || height > prevNode.Height {
		return 0, errors.New("unable to obtain previous retarget block")
	}

	firstNode := prevNode
	for ; firstNode != nil && firstNode.Height != height; firstNode = firstNode.Parent {
		// Intentionally left blank
	}

	// Limit the amount of adjustment that can occur to the previous difficulty.
	actualTimespan := int64(prevNode.Timestamp - firstNode.Timestamp)
	adjustedTimespan := actualTimespan
	if actualTimespan < minRetargetTimespan {
		adjustedTimespan = minRetargetTimespan
	} else if actualTimespan > maxRetargetTimespan {
		adjustedTimespan = maxRetargetTimespan
	}

	// Calculate new target difficulty as:
	//  currentDifficulty * (adjustedTimespan / targetTimespan)
	// The result uses integer division which means it will be slightly
	// rounded down.  Bitcoind also uses integer division to calculate this
	// result.
	oldTarget := CompactToBig(prevNode.Bits)
	newTarget := new(big.Int).Mul(oldTarget, big.NewInt(adjustedTimespan))
	newTarget.Div(newTarget, big.NewInt(targetTimespan))

	// Limit new value to the proof of work limit.
	if newTarget.Cmp(config.Parameters.ChainParam.PowLimit) > 0 {
		newTarget.Set(config.Parameters.ChainParam.PowLimit)
	}

	// Log new target difficulty and return it.  The new target logging is
	// intentionally converting the bits back to a number instead of using
	// newTarget since conversion to the compact representation loses
	// precision.
	newTargetBits := BigToCompact(newTarget)
	log.Tracef("Difficulty retarget at block height %d", prevNode.Height+1)
	log.Tracef("Old target %08x (%064x)", prevNode.Bits, oldTarget)
	log.Tracef("New target %08x (%064x)", newTargetBits, CompactToBig(newTargetBits))
	log.Tracef("Actual timespan %v, adjusted timespan %v, target timespan %v",
		time.Duration(actualTimespan)*time.Second,
		time.Duration(adjustedTimespan)*time.Second,
		config.Parameters.ChainParam.TargetTimespan)

	return newTargetBits, nil
}

func BigToCompact(n *big.Int) uint32 {
	// No need to do any work if it's zero.
	if n.Sign() == 0 {
		return 0
	}

	// Since the base for the exponent is 256, the exponent can be treated
	// as the number of bytes.  So, shift the number right or left
	// accordingly.  This is equivalent to:
	// mantissa = mantissa / 256^(exponent-3)
	var mantissa uint32
	exponent := uint(len(n.Bytes()))
	if exponent <= 3 {
		mantissa = uint32(n.Bits()[0])
		mantissa <<= 8 * (3 - exponent)
	} else {
		// Use a copy to avoid modifying the caller's original number.
		tn := new(big.Int).Set(n)
		mantissa = uint32(tn.Rsh(tn, 8*(exponent-3)).Bits()[0])
	}

	// When the mantissa already has the sign bit set, the number is too
	// large to fit into the available 23-bits, so divide the number by 256
	// and increment the exponent accordingly.
	if mantissa&0x00800000 != 0 {
		mantissa >>= 8
		exponent++
	}

	// Pack the exponent, sign bit, and mantissa into an unsigned 32-bit
	// int and return it.
	compact := uint32(exponent<<24) | mantissa
	if n.Sign() < 0 {
		compact |= 0x00800000
	}
	return compact
}

func HashToBig(hash *Uint256) *big.Int {
	// A Hash is in little-endian, but the big package wants the bytes in
	// big-endian, so reverse them.
	buf := *hash
	blen := len(buf)
	for i := 0; i < blen/2; i++ {
		buf[i], buf[blen-1-i] = buf[blen-1-i], buf[i]
	}

	return new(big.Int).SetBytes(buf[:])
}

func CompactToBig(compact uint32) *big.Int {
	// Extract the mantissa, sign bit, and exponent.
	mantissa := compact & 0x007fffff
	isNegative := compact&0x00800000 != 0
	exponent := uint(compact >> 24)

	// Since the base for the exponent is 256, the exponent can be treated
	// as the number of bytes to represent the full 256-bit number.  So,
	// treat the exponent as the number of bytes and shift the mantissa
	// right or left accordingly.  This is equivalent to:
	// N = mantissa * 256^(exponent-3)
	var bn *big.Int
	if exponent <= 3 {
		mantissa >>= 8 * (3 - exponent)
		bn = big.NewInt(int64(mantissa))
	} else {
		bn = big.NewInt(int64(mantissa))
		bn.Lsh(bn, 8*(exponent-3))
	}

	// Make it negative if the sign bit is set.
	if isNegative {
		bn = bn.Neg(bn)
	}

	return bn
}

func CalcCurrentDifficulty(currentBits uint32) string {
	var genesisBlockBits uint32 = config.Parameters.ChainParam.PowLimitBits
	targetGenesisBlockBig := CompactToBig(genesisBlockBits)
	targetCurrentBig := CompactToBig(currentBits)
	return new(big.Int).Div(targetGenesisBlockBig, targetCurrentBig).String()
}
