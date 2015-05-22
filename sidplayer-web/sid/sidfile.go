package sid

import (
	"bytes"
	"encoding/binary"
	"errors"
	"io/ioutil"
	"log"
	"os"
	"strings"
)

const (
	palHz  = 985248
	ntscHz = 1022727
)

type SidFlags struct {
	InternalPlayer bool
	ComputePlayer  bool

	C64Compatible  bool
	PlaySIDSamples bool
	BASIC_ROM      bool

	Speed    string
	SIDModel string
}

type SidFile struct {
	// 0x00, RSID / PSID (4 bytes)
	Type string

	// 0x04, RSID and PSIDv2NG == 0002
	Version uint16

	// 0x06, 0x0076 for v1, 0x007C for v2
	DataOffset uint16

	// 0x08, if this is 0 => first two bytes in dataOffset specifies loadAddress
	LoadAddress uint16

	// 0x0A, if this is 0 => equals loadAddress
	InitAddress uint16

	// 0x0C, if this is 0 => the SID will install an IRQ handler which handles playback
	PlayAddress uint16

	// 0x0E
	Songs uint16

	// 0x10
	StartSong uint16

	// 0x12, BE encoded. each bit specifies playback speed of song 1-32
	// * songs above 32 all take the 32nd bit as speed
	// * all RSIDs use 0
	// * 0 - Vertical Blank Interrupt (VBI), 50Hz PAL, 60Hz NTSC
	// * 1 - CIA#1 timer (defaults to 60Hz)
	speed uint32
	Speed [32]string

	// the following strings are all 32 bytes
	// 0x16
	Name string
	// 0x36
	Author string
	// 0x56
	Released string

	// this marks the location for the extended header for SIDv2 files
	// SIDv1 files starts the data section here

	// 0x76, BE
	// 0 - binary format
	// 		0: internal player, 1 Compute! music player
	// 1 - PlaySID samples or BASIC
	//		0: C64 Compatible, 1: contains PlaySID samples (v2NG), requires BASIC ROM (RSID)
	// 2-3: speed
	//    	0 - Unknown, 1 - PAL, 2 - NTSC, 3 - Both
	// 4-5: SID-model (v2NG)
	//    	0 - Unknown, 1 - 6581, 2 - 8580, 3 - Both
	// 6-15: Unused
	flags uint16
	Flags SidFlags

	// 0x78
	// first free page inside driver area. 0x0 means it never writes outside its
	// data area. 0xff means no available memory blocks
	StartPage uint8

	// 0x79
	// free pages after startPage. will always be 0 when startPage is 0x0 or 0xff
	PageLength uint8

	// 0x7C
	// dataarea for SIDv2 files

	// The following fields are not part of the SID header
	Hz int
}

/*
 * http://stackoverflow.com/a/13511463
 * - The values in Unicode and Latin-1 are identical (Latin-1 can be
 *   considered the 0x00 - 0xFF subset of Unicode). (...) What might confuse
 *   is the UTF-8 encoding where only 0x00 - 0x7F are encoded in the same way
 *   as Latin-1, using a single byte.
 *
 * The school-way of doing this would be to use go-charset
 * import "code.google.com/p/go-charset/charset"
 * http://godoc.org/code.google.com/p/go-charset/charset
 */
func toUtf8(iso8859_1_buf []byte) string {
	buf := make([]rune, len(iso8859_1_buf))
	for i, b := range iso8859_1_buf {
		buf[i] = rune(b)
	}

	return strings.Trim(string(buf), string(0x0))
}

func Parse(filename string) (SidFile, error) {
	s := SidFile{}

	// check filesize
	fi, err := os.Stat(filename)
	if err != nil {
		log.Println(err)
		return SidFile{}, errors.New("Failed to stat file")
	}

	if fi.Size() > 0xffff {
		return SidFile{}, errors.New("File too large")
	}

	file, err := ioutil.ReadFile(filename)
	if err != nil {
		log.Println(err)
		return SidFile{}, errors.New("Failed to open file")
	}
	bytes := bytes.NewReader(file)

	s.Type = string(file[0:4])
	if s.Type != "PSID" && s.Type != "RSID" {
		return SidFile{}, errors.New("Unknown SID format")
	}

	bytes.Seek(4, 0)
	binary.Read(bytes, binary.BigEndian, &s.Version)
	binary.Read(bytes, binary.BigEndian, &s.DataOffset)
	binary.Read(bytes, binary.BigEndian, &s.LoadAddress)
	binary.Read(bytes, binary.BigEndian, &s.InitAddress)
	binary.Read(bytes, binary.BigEndian, &s.PlayAddress)
	binary.Read(bytes, binary.BigEndian, &s.Songs)
	binary.Read(bytes, binary.BigEndian, &s.StartSong)
	binary.Read(bytes, binary.BigEndian, &s.speed)
	for i := uint(0); i < 32; i++ {
		if ((s.speed >> i) & 1) == 0 {
			s.Speed[i] = "VBI"
		} else {
			s.Speed[i] = "CIA#1"
		}
	}

	s.Name = toUtf8(file[0x16:0x35])
	s.Author = toUtf8(file[0x36:0x55])
	s.Released = toUtf8(file[0x56:0x75])

	if s.LoadAddress == 0 {
		bytes.Seek(int64(s.DataOffset), 0)
		binary.Read(bytes, binary.LittleEndian, &s.LoadAddress)
	}

	s.Hz = palHz
	/* return for v1 files */
	if s.Version == 1 {
		return s, nil
	}

	bytes.Seek(0x76, 0)
	binary.Read(bytes, binary.BigEndian, &s.flags)
	binary.Read(bytes, binary.BigEndian, &s.StartPage)
	binary.Read(bytes, binary.BigEndian, &s.PageLength)

	// 0 - binary format
	// 		0: internal player, 1 Compute! music player
	// 1 - PlaySID samples or BASIC
	//		0: C64 Compatible, 1: contains PlaySID samples (v2NG), requires BASIC ROM (RSID)
	// 2-3: speed
	//    	0 - Unknown, 1 - PAL, 2 - NTSC, 3 - Both
	// 4-5: SID-model (v2NG)
	//    	0 - Unknown, 1 - 6581, 2 - 8580, 3 - Both
	// 6-15: Unused
	if (s.flags & 0x1) == 0 {
		s.Flags.InternalPlayer = true
	} else {
		s.Flags.ComputePlayer = true
	}

	if (s.flags & 0x2) == 0 {
		s.Flags.C64Compatible = true
	} else {
		if s.Type == "RSID" {
			s.Flags.BASIC_ROM = true
		} else {
			s.Flags.PlaySIDSamples = true
		}
	}

	switch (s.flags >> 2) & 0x3 {
	case 3:
		s.Flags.Speed = "Both"
	case 2:
		s.Flags.Speed = "NTSC"
		s.Hz = ntscHz
	case 1:
		s.Flags.Speed = "PAL"
		s.Hz = palHz
	case 0:
		s.Flags.Speed = "Unknown"
	}

	switch (s.flags >> 4) & 0x3 {
	case 3:
		s.Flags.SIDModel = "Both"
	case 2:
		s.Flags.SIDModel = "8580"
	case 1:
		s.Flags.SIDModel = "6581"
	case 0:
		s.Flags.SIDModel = "Unknown"
	}

	return s, nil
}

/*
func main() {
	s, _ := Parse("/storage/C64Music/MUSICIANS/Z/Zabutom/Boten_Anna.sid")
	fmt.Printf("%#v\n", s)
}
*/
