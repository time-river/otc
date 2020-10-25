package conf

import (
	"testing"
)

func TestParseConf(t *testing.T) {
	conf, err := parseConf("example.toml")
	if err != nil {
		t.Fatal("Unexpected error:", err)
	}

	var address = ":1080"
	if conf.Server.Address != address {
		t.Fatalf("`server.address` expected '%s', but got '%s'", address, conf.Server.Address)
	}

	var upstream = []string{"http://127.0.0.1:8080"}
	if len(upstream) != len(conf.Server.Upstream) {
		t.Fatalf("`server.upstream` expected '%v', but got '%v'", upstream, conf.Server.Upstream)
	}

	var directList = []string{"example.org", "127.0.0.1"}
	if len(directList) != len(conf.Router.ProxyList) {
		t.Fatalf("`router.direct_list` expected '%v', but got '%v'", directList, conf.Router.DirectList)
	}

	var directRefs = []string{"direct-refs.txt"}
	if len(directRefs) != len(conf.Router.DirectRefs) {
		t.Fatalf("`router.direct_refs` expected '%v', but got '%v'", directRefs, conf.Router.DirectRefs)
	}
	var proxyList = []string{"example.org", "127.0.0.1"}
	if len(proxyList) != len(conf.Router.ProxyList) {
		t.Fatalf("`router.proxy_list` expected '%v', but got '%v'", proxyList, conf.Router.ProxyList)
	}

	var proxyRefs = []string{"proxy-refs.txt"}
	if len(proxyRefs) != len(conf.Router.ProxyRefs) {
		t.Fatalf("`router.proxy_refs` expected '%v', but got '%v'", proxyRefs, conf.Router.ProxyRefs)
	}
}
