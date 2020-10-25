// Package server -- receive msg then forward to upstream proxy
package server

import (
	"log"
	"net"
	"otc/conf"
)

// InitServer -- start server
func InitServer(conf *conf.Server) error {
	addr, err := net.ResolveTCPAddr("tcp", conf.Address)
	if err != nil {
		return err
	}

	server, err := net.ListenTCP("tcp", addr)
	if err != nil {
		return err
	}

	for {
		conn, err := server.AcceptTCP()
		if err != nil {
			log.Println("accept error: ", err)
			continue
		}
		log.Println(conn)
		conn.Close()
	}
}
