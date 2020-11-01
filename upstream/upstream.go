// Package upstream -- upsream handler
package upstream

import (
	"bufio"
	"bytes"
	"encoding/base64"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/url"
	"strings"
)

// Upstream -- upstream config
type Upstream struct {
	scheme   string
	hostname string
	port     string
	auth     string
}

// Parse -- parse upstream
// currently only support http
func Parse(rawurl string) (*Upstream, error) {
	u, err := url.Parse(rawurl)
	if err != nil {
		return nil, err
	}
	up := Upstream{
		scheme:   u.Scheme,
		hostname: u.Hostname(),
		port:     u.Port(),
		auth:     u.User.String(),
	}
	if !strings.EqualFold(up.scheme, "http") {
		return nil, fmt.Errorf("ONLY SUPPORT HTTP PROXY, current '%s'", up.scheme)
	}
	if len(up.port) == 0 && strings.EqualFold(up.scheme, "http") {
		up.port = "80"
	} else if len(up.port) == 0 && !strings.EqualFold(up.scheme, "http") {
		return nil, fmt.Errorf("upstream '%s' has no port", rawurl)
	}
	if len(up.auth) != 0 {
		up.auth = fmt.Sprintf("Proxy-Authorization: basic %s\r\n",
			base64.StdEncoding.EncodeToString([]byte(up.auth)))
	}

	return &up, nil
}

// CreateSession -- create upstream session
func (u *Upstream) CreateSession(origRemote *string, localConn *net.TCPConn) (*net.TCPConn, error) {
	address := u.hostname + ":" + u.port
	addr, err := net.ResolveTCPAddr("tcp", address)
	if err != nil {
		return nil, err
	}
	conn, err := net.DialTCP("tcp", nil, addr)
	if err != nil {
		return nil, err
	}

	connectHeader := buildConnectHeader(*origRemote, u.auth)
	_, err = conn.Write([]byte(connectHeader))
	if err != nil {
		conn.Close()
		return nil, err
	}

	msg, err := checkConnect(conn)
	if msg == nil && err != nil {
		conn.Close()
		return nil, err
	} else if msg != nil && err != nil {
		conn.Close()
		io.Copy(localConn, msg)
		return nil, err
	}

	return conn, nil
}

func buildConnectHeader(origRemote string, auth string) string {
	httpHeader := fmt.Sprintf(
		"CONNECT %s HTTP/1.1\r\n"+
			"Host: %s\r\n"+
			"%s"+
			"Proxy-Connection: Keep-Alive\r\n"+
			"\r\n",
		origRemote, origRemote, auth)

	return httpHeader
}

func checkConnect(conn *net.TCPConn) (*bytes.Buffer, error) {
	reader := bufio.NewReader(conn)
	resp, err := http.ReadResponse(reader, nil)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != http.StatusOK {
		msg := buildHTTPResponse(resp)
		return msg, fmt.Errorf("http response code is not 200, got: %s", resp.Status)
	}

	return nil, nil
}

func buildHTTPResponse(resp *http.Response) *bytes.Buffer {
	var buffer bytes.Buffer

	buffer.WriteString("HTTP/1.1 " + resp.Status + "\r\n")
	resp.Header.Write(&buffer)
	buffer.WriteString("\r\n")
	io.Copy(&buffer, resp.Body)

	return &buffer
}
