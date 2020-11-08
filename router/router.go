// Package router -- create packet forword rules
package router

import (
	"fmt"
	"log"
	"os"
	"os/exec"
	"os/signal"
	"strings"
	"syscall"

	"github.com/time-river/otc/conf"
)

var chainNane = "OTC_ZONE"

// Init -- create packet forward rules
func Init(conf *conf.Router) error {
	if !strings.EqualFold(conf.Mode, "local") {
		return fmt.Errorf("NOT SUPPORT %v router mode", conf.Mode)
	}

	var buffer strings.Builder
	fmt.Fprintf(&buffer, "iptables -t nat -N %s; ", chainNane)

	for _, ip := range conf.DirectList {
		fmt.Fprintf(&buffer, "iptables -t nat -A %s -d %s -j RETURN; ", chainNane, ip)
	}

	fmt.Fprintf(&buffer, "iptables -t nat -A %s -p tcp -m  multiport --dports 80,443 -j REDIRECT --to-port 1080; ", chainNane)
	fmt.Fprintf(&buffer, "iptables -t nat -A OUTPUT -p tcp -j %s; ", chainNane)

	_, err := exec.Command("/bin/bash", "-c", buffer.String()).Output()
	if err != nil {
		return err
	}

	registerCleanup()
	return nil
}

// Cleanup -- clean all rules
func Cleanup() {
	var buffer strings.Builder

	fmt.Fprintf(&buffer, "iptables -t nat -D OUTPUT -p tcp -j %s; ", chainNane)
	fmt.Fprintf(&buffer, "iptables -t nat -F %s; ", chainNane)
	fmt.Fprintf(&buffer, "iptables -t nat -X %s; ", chainNane)

	_, err := exec.Command("/bin/bash", "-c", buffer.String()).Output()
	if err != nil {
		log.Printf("exit error: %v\n", err)
	}
}

func registerCleanup() {
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		<-sigs
		Cleanup()
		os.Exit(0)
	}()
}
