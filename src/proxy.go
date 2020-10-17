package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"net/http"
	"strings"
)

// Proxy -- http proxy client struct
type Proxy struct {
	scheme  *string
	addr    *string
	port    *string
	tcpAddr *net.TCPAddr
}

// CreateSession -- create proxy session
func (p *Proxy) CreateSession(origRemote *string) (*net.TCPConn, error) {
	if p.scheme == nil || strings.Compare(*p.scheme, "http") != 0 {
		log.Println("only support http, current: ", p.scheme)
		return nil, fmt.Errorf("only support http")
	}
	conn, err := net.DialTCP("tcp", nil, p.tcpAddr)
	if err != nil {
		log.Println("TCP connect failed", err)
		return nil, err
	}

	httpHeader := fmt.Sprintf(
		"CONNECT %s HTTP/1.1\r\n"+
			"Host: %s\r\n"+
			"Proxy-Connection: Keep-Alive\r\n"+
			"\r\n",
		*origRemote, *origRemote)
	log.Println("http header:\n", httpHeader)
	conn.Write([]byte(httpHeader))
	reader := bufio.NewReader(conn)
	resp, err := http.ReadResponse(reader, nil)
	if err != nil {
		conn.Close()
		log.Println("parse http response error: ", err)
		return nil, err
	} else if resp.StatusCode != http.StatusOK {
		// TODO: write response to original requester
		conn.Close()
		log.Println("http response code is not 200, got ", resp.Status)
		return nil, fmt.Errorf("http response code is not 200, got ", resp.Status)
	}

	log.Println("http CONNECT success")
	return conn, nil
}
