package main

import (
	"net"
			"fmt"
	)

var queue []net.Conn

type messageInfo struct {
	len int
	str []byte
}

func handle(conn net.Conn) {
	queue = append(queue, conn)
	message := make(chan messageInfo, 100)
	go readInfo(conn, message)
	go writeInfo(message)
}

func readInfo(conn net.Conn, message chan messageInfo) {
	buffer := make([]byte, 255)
	defer func() {
		fmt.Println("Close One Client")
		conn.Close()
	}()

	for {
		n, err := conn.Read(buffer)
		if err == nil {
			if n > 0 {
				message <- messageInfo{n, buffer}
			}
		} else {
			break
		}
	}
}

func writeInfo(message chan messageInfo) {
	for {
		mess, ok := <- message
		if ok {

			go func(t messageInfo) {
				for _, c := range queue {
					_, err := c.Write(t.str[0:t.len])
					if err != nil {
						c.Close()
					}
				}
			}(mess)

		} else {
			break
		}
	}
}

func main() {
	ln, err := net.Listen("tcp", "127.0.0.1:55555")
	if err != nil {
		fmt.Println(err)
	} else {
		for {
			conn, err := ln.Accept()

			if err != nil {
				fmt.Println(err)
				break
			} else {
				fmt.Println("Accept One Client")
				go handle(conn)
			}
		}
	}
}
