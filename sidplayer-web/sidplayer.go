package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os/exec"
	"strings"
	"time"

	"github.com/jgilje/reveller/sidplayer-web/sid"
)

type sidplayer struct {
	load chan string
	play chan bool
	stop chan bool
	help chan bool
	song chan uint16

	Command string

	currentFile      string
	currentSong      uint16
	currentState     string
	currentSidHeader sid.SidFile

	stdin  io.WriteCloser
	reader *bufio.Reader
	stderr io.ReadCloser
}

var Sidplayer = sidplayer{
	load: make(chan string),
	play: make(chan bool),
	stop: make(chan bool),
	help: make(chan bool),
	song: make(chan uint16),
}

func (s *sidplayer) stopPlayback() error {
	_, err := io.WriteString(s.stdin, "\n")
	if err != nil {
		return err
	}

	s.currentState = "stop"
	broadCastState()

	return nil
}

func (s *sidplayer) startCmd() {
	cmd := exec.Command(s.Command)
	stdin, err := cmd.StdinPipe()
	if err != nil {
		log.Fatal(err)
	}
	s.stdin = stdin

	stderr, err := cmd.StderrPipe()
	if err != nil {
		log.Fatal(err)
	}
	s.stderr = stderr

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Fatal(err)
	}
	s.reader = bufio.NewReader(stdout)

	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}

	s.currentFile = ""
	s.currentState = "stop"
	s.currentSong = 0

	broadCastState()
}

func (s *sidplayer) run() {
	s.startCmd()

	go func() {
		for {
			line, err := s.reader.ReadString('\n')
			if err != nil {
				log.Println("stdout closed", err)
				return
			}

			if len(line) > 0 {
				fmt.Printf("sidplayer() ReadString %q\n", line)
			}
		}
	}()

	go func() {
		buf := make([]byte, 1024)
		for {
			_, err := s.stderr.Read(buf)
			if err != nil {
				log.Println("sidplayer crashed, respawning in 3 secs")
				errormsg := strings.Trim(string(buf), string(0x0))

				msg, _ := json.Marshal(ReplyMessage{MsgType: "crash", Data: errormsg})
				h.broadcast <- string(msg)

				time.Sleep(3 * time.Second)
				go s.run()
				return
			}
		}
	}()

	for {
		select {
		case file := <-s.load:
			fmt.Printf("sidplayer() loading %q\n", file)

			sidheader, err := sid.Parse(file)
			if err != nil {
				log.Println("sidplayer() failed to parse file")
				break
			}

			if s.currentState == "play" {
				if s.stopPlayback() != nil {
					return
				}
			}

			load := fmt.Sprintf("l %s\n", file)
			_, err = io.WriteString(s.stdin, load)
			if err != nil {
				log.Println("Fatal error from writing coming up! (load)")
				return
			}

			s.currentSidHeader = sidheader
			s.currentFile = strings.TrimPrefix(file, Browser.RootPath+"/")

			msg, _ := json.Marshal(ReplyMessage{MsgType: "load", Data: s.currentFile})
			h.broadcast <- string(msg)
			msg, _ = json.Marshal(sidheader)
			msg, _ = json.Marshal(ReplyMessage{MsgType: "currentSidHeader", Data: string(msg)})
			h.broadcast <- string(msg)
		case songno := <-s.song:
			fmt.Printf("sidplayer() starting subsong %q\n", songno)

			if s.currentState == "play" {
				if s.stopPlayback() != nil {
					return
				}
			}

			load := fmt.Sprintf("s %d\n", songno)
			_, err := io.WriteString(s.stdin, load)
			if err != nil {
				log.Println("Fatal error from reading coming up! (song)")
				return
			}

			if songno == 0 {
				songno = s.currentSidHeader.StartSong
			}
			s.currentSong = songno
			s.currentState = "play"

			broadCastState()
		case <-s.stop:
			if s.stopPlayback() != nil {
				return
			}
		case <-s.play:
			_, err := io.WriteString(s.stdin, "p\n")
			if err != nil {
				log.Println("Fatal error from reading coming up! (play)")
				return
			}

			broadCastState()
		case <-s.help:
			io.WriteString(s.stdin, "h\n")
		case requested_state := <-s.power:
			state := "off"
			if requested_state {
				state = "on"
			}
			power := fmt.Sprintf("power %s\n", state)
			io.WriteString(s.stdin, power)
		}
	}
}
