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
	scheme  string
	addr    string
	port    string
	auth    string
	tcpAddr *net.TCPAddr
}

// CreateSession -- create proxy session
func (p *Proxy) CreateSession(origRemote string) (*net.TCPConn, *http.Response, error) {
	var auth string = ""

	if strings.Compare(p.scheme, "http") != 0 {
		return nil, nil, fmt.Errorf("only support http, current: %s", p.scheme)
	}
	conn, err := net.DialTCP("tcp", nil, p.tcpAddr)
	if err != nil {
		return nil, nil, err
	}

	if strings.Contains(p.auth, "basic") {
		auth = "Proxy-Authorization: " + p.auth + "\r\n"
	}

	httpHeader := fmt.Sprintf(
		"CONNECT %s HTTP/1.1\r\n"+
			"Host: %s\r\n"+
			"%s"+
			"Proxy-Connection: Keep-Alive\r\n"+
			"\r\n",
		origRemote, origRemote, auth)
	log.Println("http header:\n", httpHeader)
	_, err = conn.Write([]byte(httpHeader))
	if err != nil {
		conn.Close()
		return nil, nil, err
	}

	reader := bufio.NewReader(conn)
	resp, err := http.ReadResponse(reader, nil)
	if err != nil {
		conn.Close()
		return nil, nil, err
	}

	if resp.StatusCode != http.StatusOK {
		conn.Close()
		return nil, resp, fmt.Errorf("http response code is not 200, got: %s", resp.Status)
	}

	log.Println("http CONNECT success")
	return conn, nil, nil
}
