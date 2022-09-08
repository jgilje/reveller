package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type Searcher struct {
	search  string
	prefix  string
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
		path = strings.TrimPrefix(path, s.prefix)
		s.results = append(s.results, path)
	}
	return nil
}

func (s *Searcher) SearchFile(search string, c *connection) {
	go func() {
		s.search = "*" + search + "*"

		if prefix, err := filepath.EvalSymlinks(Browser.RootPath); err != nil {
			fmt.Println(err)
			return
		} else {
			s.prefix = prefix
		}

		filepath.Walk(s.prefix, s.walker)
		fmt.Println(s.results)
		reply, _ := json.Marshal(searchReply{Results: s.results})
		msg, _ := json.Marshal(ReplyMessage{MsgType: "search", Data: string(reply)})
		c.send <- string(msg)
	}()
}
