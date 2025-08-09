package main

import (
	"flag"
	"fmt"

	"github.com/DevPutat/cryphptor/internal/scanner"
)

type arrayFlags []string

func (i *arrayFlags) String() string {
	return fmt.Sprintf("%v", *i)
}

func (i *arrayFlags) Set(value string) error {
	*i = append(*i, value)
	return nil
}

var exclude arrayFlags

func main() {
	// TODO:: cli
	p := flag.String("root", "./", "Path to root of project")
	flag.Var(&exclude, "exclude", "Exclude paths: -exclude=dir1 -exclude=dir2")
	flag.Parse()
	conf := scanner.Config{RootDir: *p, Exclude: exclude}
	fmt.Println(*p)

	s := scanner.NewScanner(conf)
	list, err := s.Scan()
	if err != nil {
		fmt.Printf("ERROR: %v", err)
		return
	}
	fmt.Println(len(list))
	if len(list) < 100 {
		fmt.Println(list)
	}
}
