// sidcontrol project main.go
package main

import (
	"flag"
	"log"
	"net/http"
	"strconv"
	"text/template"

	"golang.org/x/net/websocket"
)

var port uint
var homeTempl = template.Must(template.ParseFiles("home.html"))

var fileServer = http.FileServer(http.Dir("../www"))

func homeHandler(c http.ResponseWriter, req *http.Request) {
	if req.URL.Path == "/" {
		homeTempl.Execute(c, req.Host)
	} else {
		fileServer.ServeHTTP(c, req)
	}
}

func main() {
	flag.UintVar(&port, "port", 8080, "http service address")
	flag.StringVar(&Sidplayer.Command, "player", "sidplayer", "command for the sidplayer")
	flag.StringVar(&Browser.RootPath, "rootpath", "C64Music", "rootpath for sid files")
	flag.Parse()

	if port > 0xffff {
		log.Fatalln("Invalid port number", port)
	}

	go Sidplayer.run()
	go h.run()
	/* go broadCaster() */

	http.HandleFunc("/", homeHandler)

	ws := websocket.Server{Handler: wsHandler}
	http.Handle("/ws", ws)

	registerService(uint16(port))
	addr := ":" + strconv.FormatUint(uint64(port), 10)
	if err := http.ListenAndServe(addr, nil); err != nil {
		log.Fatal("ListenAndServe:", err)
	}
}
