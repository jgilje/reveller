// sidcontrol project main.go
package main

import (
	"code.google.com/p/go.net/websocket"
	"flag"
	"log"
	"net/http"
	"text/template"
	"time"
)

var addr = flag.String("addr", ":8080", "http service address")
var homeTempl = template.Must(template.ParseFiles("home.html"))

func homeHandler(c http.ResponseWriter, req *http.Request) {
	homeTempl.Execute(c, req.Host)
}

func broadCaster() {
	for {
		time.Sleep(3 * time.Second)
		now := time.Now().String()
		log.Println("broadcasting", now)
		h.broadcast <- now
	}
}

func main() {
	go Sidplayer.run()

	go h.run()
	/* go broadCaster() */

	http.HandleFunc("/", homeHandler)
	http.Handle("/ws", websocket.Handler(wsHandler))

	if err := http.ListenAndServe(*addr, nil); err != nil {
		log.Fatal("ListenAndServe:", err)
	}
	/*
		time.Sleep(1 * time.Second)
		Sidplayer.help <- true
		time.Sleep(1 * time.Second)
		Sidplayer.help <- true
		time.Sleep(1 * time.Second)
		log.Println("exiting")
	*/

	/*
		time.Sleep(1 * time.Second)

		stdin.Write([]byte("d\n"))
		log.Println("les")

		buf := make([]byte, 128)
		stdout.Read(buf)

		log.Println(string(buf))
	*/

	/*
		done := make(chan bool)
		go func() {
			_, err = io.Copy(os.Stdout, stdout)
			if err != nil {
				log.Fatal(err)
			}
			stdout.Close()
			done <- true
		}()
		<-done
	*/

	/*
		// reader := bufio.NewReader(stdout)
		log.Println("before")
		time.Sleep(1 * time.Second)

		buf := make([]byte, 1)
		// stdin.Write([]byte("password\n"))

		// line, err := reader.ReadString('\n')
		// line, err := reader.ReadBytes('\n')
		n, err := stdout.Read(buf)
		log.Println("read %d", n)

		fmt.Printf("Aftår readstring\n")
		time.Sleep(1 * time.Second)
		io.WriteString(stdin, "d\n")
		if err != nil {
			// You may check here if err == io.EOF
			log.Fatal(err)
		}
		fmt.Printf("%q", buf)
		fmt.Printf("%s", string(buf))
	*/

	/*
		go h.run()

		if err := http.ListenAndServe(*addr, nil); err != nil {
			log.Fatal("ListenAndServe:", err)
		}
	*/

	/*
		child, err := gexpect.Spawn("/home/jgilje/src/sidplayer/sidplayer-linux/build/sidplayer-dummy")
		if err != nil {
			panic(err)
		}
		child.Interact()
		child.Expect("6510>")
		child.SendLine("d")
		child.Close()
	*/
}
