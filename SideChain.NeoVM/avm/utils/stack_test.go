package utils

import (
	"fmt"
	"reflect"
	"runtime"
	"testing"
)

func assertEqual(t *testing.T, exp, got interface{}) {
	res := reflect.DeepEqual(exp, got)
	if res == false {
		err := fmt.Sprint("Error: expect ", exp, " got ", got)

		_, file, line, _ := runtime.Caller(1)
		t.Errorf("%s:%d %s", file, line, err)
	}
}

func TestRandomAccessStack(t *testing.T) {

	var stack = NewRandAccessStack()
	assertEqual(t, stack.Count(), 0)
	for i := 0; i < 10; i++ {
		stack.Push(i)
		assertEqual(t, stack.Count(), i+1)
	}

	for i := 9; i >= 0; i-- {
		elem := stack.Pop()
		assertEqual(t, elem.(int), i)
	}
	assertEqual(t, stack.Count(), 0)

	for i := 0; i < 10; i++ {
		stack.Insert(i, i)
		assertEqual(t, stack.Peek(i).(int), i)
		assertEqual(t, stack.Peek(0).(int), 0)
	}

	for i := 0; i < 10; i++ {
		stack.Set(i, i+1)
		assertEqual(t, stack.Peek(i).(int), i+1)
	}

}

func TestRandomAccessStack_CopyTo(t *testing.T) {
	var stack = NewRandAccessStack()
	for i := 0; i < 10; i++ {
		stack.Push(i)
	}

	var toStack = NewRandAccessStack()
	stack.CopyTo(toStack, -1)
	for i := 0; i < 10; i++ {
		assertEqual(t, stack.Peek(i), toStack.Peek(i))
	}

	toStack.Clear()
	assertEqual(t, toStack.Count(), 0)

	stack.CopyTo(toStack, 0)
	assertEqual(t, toStack.Count(), 0)

	stack.CopyTo(toStack, 0)
	for i := 0; i < toStack.Count(); i++ {
		fmt.Print(toStack.Peek(i))
	}
}

func TestRandomAccessStack_Swap(t *testing.T) {
	var stack = NewRandAccessStack()
	for i := 0; i < 10; i++ {
		stack.Push(i)
	}
	for i := 0; i < stack.Count(); i++ {
		fmt.Print(stack.Peek(i))
	}
	fmt.Println("\n")
	stack.Swap(1,5)

	for i := 0; i < stack.Count(); i++ {
		fmt.Print(stack.Peek(i))
	}

}
