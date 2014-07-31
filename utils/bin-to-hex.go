// bin-to-hex
// http://www.zimmers.net/anonftp/pub/cbm/firmware/computers/c64/
package main

import (
	"bytes"
	"errors"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
)

func main() {
	filename := flag.String("file", "file.bin", "input file")
	flag.Parse()

	fi, err := os.Stat(*filename)
	if err != nil {
		log.Fatal(err)
	}

	if fi.Size() > 0xffff {
		log.Fatal(errors.New("File too large"))
	}

	file, err := ioutil.ReadFile(*filename)
	if err != nil {
		log.Fatal(err)
	}

	bytes := bytes.NewReader(file)
	for i := int64(0); i < fi.Size(); i++ {
		if i%8 == 0 {
			fmt.Print("\n ")
		}

		b, err := bytes.ReadByte()
		if err != nil {
			log.Fatal(err)
		}

		fmt.Printf("0x%02x, ", b)
	}
	fmt.Println()
}
