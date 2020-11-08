// Package conf -- parse toml configuration file
package conf

import (
	"fmt"
	"log"

	"github.com/pelletier/go-toml"
	cli "github.com/urfave/cli/v2"
)

// Server -- server config
type Server struct {
	Address  string `toml:"address"`
	Upstream string `toml:"upstream"`
}

// Router -- forward rules
type Router struct {
	Mode       string
	DirectList []string `toml:"direct_list"`
	DirectRefs []string `toml:"direct_refs"`
	ProxyList  []string `toml:"proxy_list"`
	ProxyRefs  []string `toml:"proxy_refs"`
}

// Conf -- configuration struct
type Conf struct {
	Server
	Router
}

func parseConf(path string) (*Conf, error) {
	var conf Conf

	config, err := toml.LoadFile(path)
	if err != nil {
		return nil, err
	}
	err = config.Unmarshal(&conf)
	if err != nil {
		return nil, err
	}
	return &conf, nil
}

// ParseCmd -- parse config from file or cmdline
func ParseCmd(c *cli.Context) (*Conf, error) {
	var conf Conf

	confPath := c.String("config")
	if len(confPath) != 0 {
		log.Printf("parse parameter from config file '%s'\n", confPath)
		temp, err := parseConf(confPath)
		if err != nil {
			return nil, err
		}

		conf = *temp
	} else {
		// TODO:
		return nil, fmt.Errorf("NOT SUPPORT PARSE conf FROM CMDLINE")
	}

	return &conf, nil
}
