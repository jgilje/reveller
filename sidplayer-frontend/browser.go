package main

import (
	"errors"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

var rootpath string = "/net/xerxes/storage/C64Music"
var walkerFinished = make(chan bool)

type browser struct {
}

func walker(path string, info os.FileInfo, err error) error {
	log.Println(path)
	return nil
}

func notUsed() {
	filepath.Walk("/net/xerxes/storage/C64Music", walker)
}

func SidPath(path string) (string, error) {
	file := filepath.Join(rootpath, path)
	if !strings.HasPrefix(file, rootpath) {
		log.Println("Trying to escape rootpath")
		return "", errors.New("Trying to escape rootpath")
	}

	ext := strings.ToLower(filepath.Ext(file))
	if ext != ".sid" {
		log.Println("Wrong extension:", ext)
		return "", errors.New("Wrong extension")
	}

	if _, err := os.Stat(file); os.IsNotExist(err) {
		log.Println("File not found:", file)
		return "", errors.New("File not found")
	}

	return file, nil
}

func Readpath(path string) (subdirs []string, sidfiles []string) {
	dir := filepath.Join(rootpath, path)
	if !strings.HasPrefix(dir, rootpath) {
		log.Fatal("Trying to escape rootpath")
		return
	}

	files, err := ioutil.ReadDir(dir)
	if err != nil {
		log.Fatal(err)
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
