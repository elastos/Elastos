package utils

import (
	"fmt"
	"os"

	"github.com/howeyc/gopass"
)

// GetPassword gets password from user input
func GetPassword() ([]byte, error) {
	fmt.Printf("Password:")
	return gopass.GetPasswd()
}

// GetConfirmedPassword gets double confirmed password from user input
func GetConfirmedPassword() ([]byte, error) {
	fmt.Printf("Password:")
	first, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}
	fmt.Printf("Re-enter Password:")
	second, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}
	if len(first) != len(second) {
		fmt.Println("Unmatched Password")
		os.Exit(1)
	}
	for i, v := range first {
		if v != second[i] {
			fmt.Println("Unmatched Password")
			os.Exit(1)
		}
	}
	return first, nil
}

func FileExisted(filename string) bool {
	_, err := os.Stat(filename)
	return err == nil || os.IsExist(err)
}
