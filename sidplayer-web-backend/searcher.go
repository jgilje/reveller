package main

import (
	"encoding/json"
	"fmt"
	"io/fs"
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

func (s *Searcher) walker(path string, d fs.DirEntry, err error) error {
	if err != nil {
		return nil
	}
	if d.IsDir() {
		return nil
	}
	matched, err := filepath.Match(s.search, strings.ToLower(d.Name()))
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
		s.search = "*" + strings.ToLower(search) + "*"

		if prefix, err := filepath.EvalSymlinks(Browser.RootPath); err != nil {
			fmt.Println(err)
			return
		} else {
			s.prefix = prefix
		}

		filepath.WalkDir(s.prefix, s.walker)
		reply, _ := json.Marshal(searchReply{Results: s.results})
		msg, _ := json.Marshal(ReplyMessage{MsgType: "search", Data: string(reply)})

		select {
		case <-c.Done():
			return
		default:
			c.send <- string(msg)
		}
	}()
}
