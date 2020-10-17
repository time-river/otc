package main

import (
	"fmt"
	"net"
	"syscall"
	"unsafe"
)

const SO_ORIGINAL_DST = 80

func main() {
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
		go connHandler(conn)
	}
}

func connHandler(leftConn *net.TCPConn) {
	origAddr := getOrigAddr(leftConn)

}

func getOrigAddr(leftConn *net.TCPConn) (net.TCPAddr, error) {
	var origAddr net.TCPAddr

	leftFile, err := leftConn.File()
	if err != nil {
		fmt.Println("leftConn.File error: %w", err)
		return origAddr, err
	}
	addr, err := syscall.GetsockoptIPv6Mreq(int(leftFile.Fd()), syscall.IPPROTO_IP, SO_ORIGINAL_DST)
	if err != nil {
		fmt.Println("syscall.GetsockoptIPv6Mreq error: %w", err)
		return origAddr, err
	}
	origConn := (*syscall.RawSockaddrInet4)(unsafe.Pointer(addr))

	origAddr.IP = net.IPv4(origConn.Addr[0], origConn.Addr[1], origConn.Addr[2], origConn.Addr[3])
	origAddr.Port = int(origConn.Port)

	return origAddr, nil
}
