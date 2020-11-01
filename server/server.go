// Package server -- receive msg then forward to upstream proxy
package server

import (
	"fmt"
	"io"
	"log"
	"net"
	"os"
	"sync"
	"syscall"

	"otc/conf"
	"otc/upstream"
)

const SO_ORIGINAL_DST = 80

// Run -- start server
func Run(conf *conf.Server) error {
	up, err := upstream.Parse(conf.Upstream)
	if err != nil {
		return err
	}

	// TODO: upstream connection test

	addr, err := net.ResolveTCPAddr("tcp", conf.Address)
	if err != nil {
		return err
	}
	server, err := net.ListenTCP("tcp", addr)
	if err != nil {
		return err
	}

	log.Printf("Server run, listen '%s' ...\n", conf.Address)
	for {
		conn, err := server.AcceptTCP()
		if err != nil {
			log.Println("accept error: ", err)
			continue
		}
		go connHandler(conn, up)
	}
}

func connHandler(localConn *net.TCPConn, up *upstream.Upstream) {
	defer localConn.Close()

	connFile, err := localConn.File()
	if err != nil {
		log.Println("leftConn.File: ", err)
		return
	}

	origRemote, err := getOrigAddr(connFile)
	if err != nil {
		log.Println("getOrigAddr: ", err)
		return
	}
	remoteConn, err := up.CreateSession(origRemote, localConn)
	if err != nil {
		log.Printf("CreateSesson('%s') failed: %v\n", *origRemote, err)
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

func getOrigAddr(file *os.File) (*string, error) {
	addr, err := syscall.GetsockoptIPv6Mreq(int(file.Fd()), syscall.IPPROTO_IP, SO_ORIGINAL_DST)
	if err != nil {
		return nil, err
	}

	remote := fmt.Sprintf("%d.%d.%d.%d:%d",
		addr.Multiaddr[4], addr.Multiaddr[5], addr.Multiaddr[6], addr.Multiaddr[7],
		uint16(addr.Multiaddr[2])<<8+uint16(addr.Multiaddr[3]))

	return &remote, nil
}
