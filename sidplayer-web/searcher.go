package main

import (
	"os"
	"fmt"
	"path/filepath"
	"encoding/json"
)

type Searcher struct {
	search string
	results []string
}

type searchReply struct {
	Results []string `json:"results"`
}

func (s *Searcher) walker(path string, info os.FileInfo, err error) error {
	if err != nil {
        return nil
    }
    if info.IsDir() {
        return nil
    }
    matched, err := filepath.Match(s.search, info.Name())
    if err != nil {
        return err
    }
    if matched {
        fmt.Println(path)
		s.results = append(s.results, path)
    }
    return nil
}

func (s *Searcher) SearchFile(search string, c *connection) {
	go func() {
		s.search = "*" + search + "*"
		filepath.Walk(Browser.RootPath, s.walker)
		fmt.Println(s.results)
		reply, _ := json.Marshal(searchReply{Results: s.results})
		msg, _ := json.Marshal(ReplyMessage{MsgType: "search", Data: string(reply)})
		c.send <- string(msg)
	}()
}
