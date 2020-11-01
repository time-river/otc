// Package router -- create packet forword rules
package router

import (
	"otc/conf"

	"github.com/google/nftables"
)

var chainNane = "OTC_ZONE"

func buildRules(table *nftables.Table) *nftables.Rule {

	return nil
}

// InitRouter -- create packet forward rules
func InitRouter(conf *conf.Router) error {
	/*
		conn := &nftables.Conn{NetNS: 0}

		natTable := &nftables.Table{
			Name:   "nat",
			Family: nftables.TableFamilyIPv4,
		}

		otcChain := conn.AddChain(&nftables.Chain{
			Name:     chainNane,
			Hooknum:  nftables.ChainHookPrerouting,
			Priority: nftables.ChainPriorityNATDest,
			Table:    natTable,
			Type:     nftables.ChainTypeNAT,
		})
		log.Println(otcChain)
	*/

	return nil
}

// CleanRouter -- clean packet forward rules
func CleanRouter() error {
	//conn := &nftables.Conn{NetNS: 0}
	return nil
}
