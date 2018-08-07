package main

import (
	"net"
	"os"
	"strconv"
	"fmt"
	"time"
)

func read(ln net.Conn, uid int) {
	buffer := make([]byte, 255)
	fp, err := os.Create("./info_" + strconv.Itoa(uid) + ".txt")
	defer fp.Close()
	if err == nil {
		for {
			n, err := ln.Read(buffer)

			if err == nil && n > 0 {
				uidStr := strconv.Itoa(uid)

				if uidStr == string(buffer[0:len(uidStr)]) {
					continue
				}

				fp.Write(buffer[0:n])
				fp.Write([]byte("\n"))
			} else {
				break
			}

		}
	}
}

func write(ln net.Conn, finish chan bool, uid int) {
	var mess string
	defer func(){
		finish <- true
	}()
	for {
		fmt.Print("Message : ")
		fmt.Scanf("%s\n", &mess)

		if mess == "close" {
			break
		} else {
			str := fmt.Sprintf("%d:%s:%s", uid, time.Now(), mess)
			_, err := ln.Write([]byte(str))
			if err == nil {
				fmt.Println("send success")
			} else {
				break
			}
		}
	}

}

func main() {
	uid := 0
	finish := make(chan bool)
	fmt.Print("Enter your ID: ")
	fmt.Scanf("%d\n", &uid)

	ln, err := net.Dial("tcp", "127.0.0.1:55555")
	if err != nil {
		fmt.Println(err)
	} else {
		go read(ln, uid)
		go write(ln, finish, uid)
		<- finish
		ln.Close()
	}
}
