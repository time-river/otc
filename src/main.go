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
	tcpAddr, err := net.ResolveTCPAddr("tcp", paddr+":"+pport)
	if err != nil {
		panic(err)
	}
	proxy := &Proxy{
		scheme:  &pscheme,
		addr:    &paddr,
		port:    &pport,
		tcpAddr: tcpAddr,
	}
	addr, err := net.ResolveTCPAddr("tcp", ":1080")
	if err != nil {
		panic(err)
	}
	server, err := net.ListenTCP("tcp", addr)
	if err != nil {
		panic(err)
	}

	for {
		conn, err := server.AcceptTCP()
		if err != nil {
			fmt.Println("accept error: %w", err)
			continue
		}
		go connHandler(conn, proxy)
	}
}

func connHandler(localConn *net.TCPConn, proxy *Proxy) {
	defer localConn.Close()
	connFile, err := localConn.File()
	if err != nil {
		fmt.Println("leftConn.File error: %w", err)
		return
	}

	fmt.Println("......")
	origRemote, err := getOrigAddr(connFile)
	if err != nil {
		fmt.Println("getOrigAddr")
		return
	}
	remoteConn, err := proxy.CreateSession(origRemote)
	if err != nil {
		return
	}
	defer remoteConn.Close()

	log.Println("remoteConn: ", remoteConn)
	var streamWait sync.WaitGroup
	streamWait.Add(2)

	streamConn := func(dst io.Writer, src io.Reader) {
		io.Copy(dst, src)
		streamWait.Done()
	}

	go streamConn(remoteConn, localConn)
	go streamConn(localConn, remoteConn)

	streamWait.Wait()
}

func getOrigAddr(file *os.File) (*string, error) {
	addr, err := syscall.GetsockoptIPv6Mreq(int(file.Fd()), syscall.IPPROTO_IP, SO_ORIGINAL_DST)
	if err != nil {
		fmt.Println("syscall.GetsockoptIPv6Mreq error: %w", err)
		return nil, err
	}

	remote := fmt.Sprintf("%d.%d.%d.%d:%d",
		addr.Multiaddr[4], addr.Multiaddr[5], addr.Multiaddr[6], addr.Multiaddr[7],
		uint16(addr.Multiaddr[2])<<8+uint16(addr.Multiaddr[3]))
	log.Println("connect: ", remote)

	return &remote, nil
}
