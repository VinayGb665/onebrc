package main

import (
	"bufio"
	"fmt"
	"io"
	"os"
	"sync"
)

type chunk struct {
	begin, end int64
}
type task struct {
	reader    io.Reader
	linesRead int64
}

func (t *task) run() {
	scanner := bufio.NewScanner(t.reader)
	counter := int64(0)
	for scanner.Scan() {
		scanner.Bytes()
		counter++
	}
	t.linesRead = counter
	assert(scanner.Err())
}

func createTask(element chunk, path string) *task {
	fp, err := os.Open(path)
	assert(err)
	_, err = fp.Seek(element.begin, 0)
	assert(err)

	sr := io.NewSectionReader(fp, element.begin, element.end-element.begin)
	br := bufio.NewReader(sr)

	return &task{br, 0}
}
func assert(err error) {
	if err != nil {
		panic(err)
	}
	return
}

func main() {
	fmt.Println("Fo go")
	runner("../data.txt", 8)

}

func executeTasks(tasks []*task) {
	var wg sync.WaitGroup
	for _, task := range tasks {
		t := task
		wg.Add(1)
		go func() {
			defer wg.Done()
			t.run()
		}()
	}
	wg.Wait()
	totalLines := int64(0)
	for _, task := range tasks {
		totalLines += task.linesRead
	}
	fmt.Printf("Damn I read %d lines", totalLines)
}

func runner(path string, workers int64) {

	fp, err := os.Open(path)
	if err != nil {
		panic("Can't open")
	}

	stats, err := fp.Stat()
	assert(err)
	size := stats.Size()
	chunkSize := size / workers

	var tasks []*task

	base := int64(0)

	for {
		chunkEnd := base + chunkSize
		if chunkEnd > size {
			task := createTask(chunk{base, size}, path)
			tasks = append(tasks, task)
			break
		}

		_, err := fp.Seek(int64(chunkEnd), 0)
		assert(err)

		scanner := bufio.NewScanner(bufio.NewReader(fp))
		scanner.Split(bufio.ScanBytes)

		for scanner.Scan() {
			chunkEnd++
			if scanner.Bytes()[0] == '\n' {
				break
			}
		}
		task := createTask(chunk{base, chunkEnd}, path)
		tasks = append(tasks, task)

		base = chunkEnd
	}
	executeTasks(tasks)
}
