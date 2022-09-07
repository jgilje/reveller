package main

import (
	"errors"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

type browser struct {
	RootPath string
}

var Browser = browser{}

func SidPath(path string) (string, error) {
	file := filepath.Join(Browser.RootPath, path)
	if !strings.HasPrefix(file, Browser.RootPath) {
		log.Println("Trying to escape rootpath")
		return "", errors.New("trying to escape rootpath")
	}

	ext := strings.ToLower(filepath.Ext(file))
	if ext != ".sid" {
		log.Println("Wrong extension:", ext)
		return "", errors.New("wrong extension")
	}

	if _, err := os.Stat(file); os.IsNotExist(err) {
		log.Println("File not found:", file)
		return "", errors.New("file not found")
	}

	return file, nil
}

func Readpath(path string) (subdirs []string, sidfiles []string) {
	dir := filepath.Join(Browser.RootPath, path)
	if !strings.HasPrefix(dir, Browser.RootPath) {
		log.Println("Trying to escape rootpath")
		return
	}

	files, err := ioutil.ReadDir(dir)
	if err != nil {
		log.Println(err)
		return
	}

	dirs := []string{}
	sids := []string{}
	for _, v := range files {
		if v.IsDir() {
			dirs = append(dirs, v.Name())
		} else {
			if match, err := filepath.Match("*.sid", v.Name()); match && err == nil {
				sids = append(sids, v.Name())
			}
		}
	}

	return dirs, sids
}
