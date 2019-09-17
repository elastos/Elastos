package base64url

import (
	"strings"
	"testing"

	"github.com/stretchr/testify/assert"
)

func Test_DecodeString(t *testing.T) {

	//no _ and -
	No := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDIIVpLKGGCgjoUdQ"
	data, err := DecodeString(No)
	assert.True(t, err == nil)
	assert.True(t, len(data) == 64)

	//one - and one _
	Two := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDI-VpLK_GCgjoUdQ"
	data, err = DecodeString(Two)
	assert.True(t, err == nil)
	assert.True(t, len(data) == 64)

	OneMinus := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDI-VpLKGGCgjoUdQ"
	data, err = DecodeString(OneMinus)
	assert.True(t, err == nil)
	assert.True(t, len(data) == 64)

	One_ := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDIVVpLK_GCgjoUdQ"
	data, err = DecodeString(One_)
	assert.True(t, err == nil)
	assert.True(t, len(data) == 64)

	//more _ and -
	More := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUi__W82jW4xNlv--EDIIVpLKGGCgjoUdQ"
	data, err = DecodeString(More)
	assert.True(t, err == nil)
	assert.True(t, len(data) == 64)
}

func Test_EncodeToString(t *testing.T) {

	//no _ and -
	No := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDIIVpLKGGCgjoUdQ"
	data, _ := DecodeString(No)
	No2 := EncodeToString(data)
	No2Trim := strings.TrimRight(No2, "=")
	assert.True(t, No == No2Trim)

	//one - and one _
	Two := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDI-VpLK_GCgjoUdQ"
	data, _ = DecodeString(Two)
	Two2 := EncodeToString(data)
	Two2Trim := strings.TrimRight(Two2, "=")
	assert.True(t, strings.TrimRight(Two2, "=") == Two2Trim)

	OneMinus := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDI-VpLKGGCgjoUdQ"
	data, _ = DecodeString(OneMinus)
	OneMinus2 := EncodeToString(data)
	OneMinus2Trim := strings.TrimRight(OneMinus2, "=")
	assert.True(t, strings.TrimRight(OneMinus2, "=") == OneMinus2Trim)

	One_ := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDIVVpLK_GCgjoUdQ"
	data, _ = DecodeString(One_)
	One_2 := EncodeToString(data)
	One_2Trim := strings.TrimRight(One_2, "=")
	assert.True(t, strings.TrimRight(One_2, "=") == One_2Trim)

	//more _ and -
	More := "nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUi__W82jW4xNlv--EDIIVpLKGGCgjoUdQ"
	data, _ = DecodeString(More)
	More_2 := EncodeToString(data)
	More_2Trim := strings.TrimRight(More_2, "=")
	assert.True(t, strings.TrimRight(More_2, "=") == More_2Trim)
}
