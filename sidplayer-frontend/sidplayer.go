package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os/exec"
	"strings"
)

type sidplayer struct {
	load chan string
	play chan bool
	stop chan bool
	help chan bool
	song chan int8

	Command string

	currentFile  string
	currentSong  int8
	currentState string

	stdin io.WriteCloser
}

var Sidplayer = sidplayer{
	load: make(chan string),
	play: make(chan bool),
	stop: make(chan bool),
	help: make(chan bool),
	song: make(chan int8),
}

func (s *sidplayer) stopPlayback() {
	_, err := io.WriteString(s.stdin, "\n")
	if err != nil {
		log.Fatal(err)
	}

	Sidplayer.currentState = "stop"

	msg, _ := json.Marshal(ReplyMessage{MsgType: "stateChange", Data: "stop"})
	h.broadcast <- string(msg)
}

func (s *sidplayer) run() {
	cmd := exec.Command(s.Command)
	// cmd := exec.Command("stdbuf", "-oL", "-e0", "/home/jgilje/src/sidplayer/sidplayer-linux/build/sidplayer-dummy")
	stdin, err := cmd.StdinPipe()
	if err != nil {
		log.Fatal(err)
	}
	s.stdin = stdin

	stdout, err := cmd.StdoutPipe()
	if err != nil {
		log.Fatal(err)
	}
	reader := bufio.NewReader(stdout)

	if err := cmd.Start(); err != nil {
		log.Fatal(err)
	}

	/*
		go func() {
			_, err := io.WriteString(stdin, "d\n")
			if err != nil {
				log.Fatal(err)
			}
		}()
		done := make(chan bool)
		go func() {
			for {
				line, err := reader.ReadString('\n')
				if err != nil {
					log.Println(err)
					done <- true
				}
				log.Println(line)
			}
		}()
	*/
	Sidplayer.currentState = "stop"

	go func() {
		for {
			line, err := reader.ReadString('\n')
			if err != nil && err != io.EOF {
				log.Fatal(err)
			}
			fmt.Printf("sidplayer() ReadString %q\n", line)
		}
	}()

	for {
		select {
		case file := <-s.load:
			fmt.Printf("sidplayer() loading %q\n", file)

			if Sidplayer.currentState == "play" {
				s.stopPlayback()
			}

			load := fmt.Sprintf("l %s\n", file)
			_, err := io.WriteString(stdin, load)
			if err != nil {
				log.Fatal(err)
			}

			Sidplayer.currentFile = strings.TrimPrefix(file, Browser.RootPath+"/")

			msg, _ := json.Marshal(ReplyMessage{MsgType: "load", Data: Sidplayer.currentFile})
			h.broadcast <- string(msg)
		case songno := <-s.song:
			fmt.Printf("sidplayer() starting subsong %q\n", songno)
			load := fmt.Sprintf("s %d\n", songno)
			_, err := io.WriteString(stdin, load)
			if err != nil {
				log.Fatal(err)
			}

			Sidplayer.currentSong = songno
			Sidplayer.currentState = "play"

			msg, _ := json.Marshal(ReplyMessage{MsgType: "stateChange", Data: "play"})
			h.broadcast <- string(msg)
		case <-s.stop:
			s.stopPlayback()
		case <-s.play:
			_, err := io.WriteString(stdin, "p\n")
			if err != nil {
				log.Fatal(err)
			}

			Sidplayer.currentState = "play"

			msg, _ := json.Marshal(ReplyMessage{MsgType: "stateChange", Data: "play"})
			h.broadcast <- string(msg)
		case <-s.help:
			io.WriteString(stdin, "h\n")
		}
	}
}
