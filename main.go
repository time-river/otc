package main

import (
	"log"
	"os"

	cli "github.com/urfave/cli/v2"

	"github.com/time-river/otc/conf"
	"github.com/time-river/otc/router"
	"github.com/time-river/otc/server"
)

func action(c *cli.Context) error {
	conf, err := conf.ParseCmd(c)
	if err != nil {
		return err
	}

	err = router.Init(&conf.Router)
	if err != nil {
		return err
	}
	err = server.Run(&conf.Server)
	if err != nil {
		router.Cleanup()
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
