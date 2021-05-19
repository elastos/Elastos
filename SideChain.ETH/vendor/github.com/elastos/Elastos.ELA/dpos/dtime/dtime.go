package dtime

import "time"

// Time duration constants.
const (
	Nanosecond  int64 = 1
	Microsecond       = 1000 * Nanosecond
	Millisecond       = 1000 * Microsecond
	Second            = 1000 * Millisecond
)

// Now returns current time in million second precision.
func Now() time.Time {
	return Int64ToTime(time.Now().UnixNano())
}

// int64ToTime creates a UNIX time in million second precision by the given
// nano seconds.
func Int64ToTime(nanosec int64) time.Time {
	return time.Unix(nanosec/Second, nanosec%Second/Millisecond*Millisecond)
}
