package main

import (
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"sync"
	"syscall"
)

const SO_ORIGINAL_DST = 80

func main() {
	pscheme := "http"
	paddr := "127.0.0.1"
	pport := "3128"
	pauth := "basic YWxhZGRpbjpvcGVuc2VzYW1l"
	tcpAddr, err := net.ResolveTCPAddr("tcp", paddr+":"+pport)
	if err != nil {
		panic(err)
	}
	proxy := &Proxy{
		scheme:  pscheme,
		addr:    paddr,
		port:    pport,
		auth:    pauth,
		tcpAddr: tcpAddr,
	}
	addr, err := net.ResolveTCPAddr("tcp", ":1080")
	if err != nil {
		log.Panicln(err)
	}
	server, err := net.ListenTCP("tcp", addr)
	if err != nil {
		log.Panicln(err)
	}

	for {
		conn, err := server.AcceptTCP()
		if err != nil {
			log.Println("accept error: ", err)
			continue
		}
		go connHandler(conn, proxy)
	}
}

func connHandler(localConn *net.TCPConn, proxy *Proxy) {
	defer localConn.Close()

	connFile, err := localConn.File()
	if err != nil {
		log.Println("leftConn.File: ", err)
		return
	}

	fmt.Println("......")
	origRemote, err := getOrigAddr(connFile)
	if err != nil {
		log.Println("getOrigAddr: ", err)
		return
	}
	remoteConn, resp, err := proxy.CreateSession(origRemote)
	if err != nil && resp == nil {
		log.Println("CreateSession: ", err)
		return
	} else if err != nil && resp != nil {
		log.Println("CreateSession: ", err)
		statusCode := []byte("HTTP/1.1 " + resp.Status + "\r\n")
		localConn.Write(statusCode)
		resp.Header.Write(localConn)
		localConn.Write([]byte{'\r', '\n'})
		io.Copy(localConn, resp.Body)
		return
	}
	defer remoteConn.Close()

	var streamWait sync.WaitGroup
	streamWait.Add(2)

	streamConn := func(dst io.Writer, src io.Reader) {
		_, err := io.Copy(dst, src)
		if err != nil {
			log.Panicln("streamConn: ", err)
		}
		streamWait.Done()
	}

	go streamConn(remoteConn, localConn)
	go streamConn(localConn, remoteConn)

	streamWait.Wait()
}

func getOrigAddr(file *os.File) (string, error) {
	addr, err := syscall.GetsockoptIPv6Mreq(int(file.Fd()), syscall.IPPROTO_IP, SO_ORIGINAL_DST)
	if err != nil {
		log.Println("syscall.GetsockoptIPv6Mreq error: %w", err)
		return "", err
	}

	remote := fmt.Sprintf("%d.%d.%d.%d:%d",
		addr.Multiaddr[4], addr.Multiaddr[5], addr.Multiaddr[6], addr.Multiaddr[7],
		uint16(addr.Multiaddr[2])<<8+uint16(addr.Multiaddr[3]))
	log.Println("remote: ", remote)

	return remote, nil
}
