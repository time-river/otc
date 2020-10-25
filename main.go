package main

import (
	"log"
	"os"

	"github.com/urfave/cli/v2" // imports as package "cli"

	"otc/conf"
	"otc/router"
	"otc/server"
)

func action(c *cli.Context) error {
	conf, err := conf.ParseCmd(c)
	if err != nil {
		return err
	}

	err = router.InitRouter(&conf.Router)
	if err != nil {
		return err
	}
	err = server.InitServer(&conf.Server)
	if err != nil {
		router.CleanRouter()
		return err
	}
	return nil
}

func main() {
	app := &cli.App{
		Flags: []cli.Flag{
			&cli.StringFlag{
				Name:    "config",
				Aliases: []string{"c"},
				Usage:   "Load configuration from `FILE`",
			},
			&cli.StringFlag{
				Name:    "proxy",
				Aliases: []string{"p"},
				Usage:   "Use the specified proxy",
			},
		},
		Action: action,
	}

	err := app.Run(os.Args)
	if err != nil {
		log.Fatal(err)
	}
}
