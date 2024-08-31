package main

import (
	"context"
	"encoding/json"
	"log"
	"strconv"

	"golang.org/x/net/websocket"
)

type connection struct {
	// The websocket connection.
	ws *websocket.Conn

	// Buffered channel of outbound messages.
	send chan string

	context.Context
}

type SidAction struct {
	Action   string `json:"action"`
	Argument string `json:"argument"`
}

type ReplyMessage struct {
	MsgType string `json:"type"`
	Data    string `json:"data"`
}

type lsReply struct {
	Path        string   `json:"path"`
	Directories []string `json:"directories"`
	SidFiles    []string `json:"sidfiles"`
}

type StateReply struct {
	File  string    `json:"file"`
	State PlayState `json:"state"`
	Song  uint16    `json:"song"`
	Power bool      `json:"power"`
}

func broadCastState() {
	s := StateReply{File: Sidplayer.currentFile, State: Sidplayer.currentState, Song: Sidplayer.currentSong, Power: Sidplayer.currentPower}
	msg, _ := json.Marshal(s)
	reply := ReplyMessage{MsgType: "state", Data: string(msg)}
	msg, _ = json.Marshal(reply)
	h.broadcast <- string(msg)
}

func (c *connection) reader() {
	for {
		/*
			var message string
			err := websocket.Message.Receive(c.ws, &message)
			if err != nil {
				break
			}


			h.broadcast <- message
		*/

		var action SidAction
		err := websocket.JSON.Receive(c.ws, &action)
		if err != nil {
			log.Println(err)
			break
		}

		switch action.Action {
		case "ls":
			dirs, sids := Readpath(action.Argument)
			lsreply := lsReply{Path: action.Argument, Directories: dirs, SidFiles: sids}
			msg, _ := json.Marshal(lsreply)
			reply := ReplyMessage{MsgType: action.Action, Data: string(msg)}
			websocket.JSON.Send(c.ws, reply)
		case "search":
			path := action.Argument
			searcher := Searcher{}
			searcher.SearchFile(path, c)
		case "load":
			file, err := SidPath(action.Argument)
			if err != nil {
				websocket.JSON.Send(c.ws, "err")
				break
			}
			Sidplayer.load <- file
		case "song":
			song, err := strconv.ParseInt(action.Argument, 0, 8)
			if err != nil {
				websocket.JSON.Send(c.ws, err)
				break
			}
			Sidplayer.song <- uint16(song)
		case "stop":
			Sidplayer.stop <- true
		case "play":
			Sidplayer.play <- true
		case "state":
			s := StateReply{File: Sidplayer.currentFile, State: Sidplayer.currentState, Song: Sidplayer.currentSong, Power: Sidplayer.currentPower}
			msg, _ := json.Marshal(s)
			reply := ReplyMessage{MsgType: action.Action, Data: string(msg)}
			websocket.JSON.Send(c.ws, reply)
		case "currentHeader":
			msg, _ := json.Marshal(Sidplayer.currentSidHeader)
			reply := ReplyMessage{MsgType: "currentSidHeader", Data: string(msg)}
			websocket.JSON.Send(c.ws, reply)
		case "power":
			if state, err := strconv.ParseBool(action.Argument); err == nil {
				Sidplayer.power <- state
			}
		default:
			websocket.JSON.Send(c.ws, "unknown action")
		}
	}
	c.ws.Close()
}

func (c *connection) writer() {
	for message := range c.send {
		err := websocket.Message.Send(c.ws, message)
		if err != nil {
			break
		}
	}
	c.ws.Close()
}

func wsHandler(ws *websocket.Conn) {
	ctx := context.Background()
	ctxWithCancel, cancel := context.WithCancel(ctx)

	c := &connection{send: make(chan string, 256), ws: ws, Context: ctxWithCancel}
	h.register <- c
	defer func() {
		cancel()
		h.unregister <- c
	}()
	go c.writer()
	c.reader()
}
