package main

import (
	"fmt"
	"github.com/godbus/dbus"
	"github.com/oleksandr/bonjour"
	"log"
	"os"
)

const AVAHI_DBUS_NAME string = "org.freedesktop.Avahi"
const AVAHI_INTERFACE_SERVER string = AVAHI_DBUS_NAME + ".Server"
const AVAHI_INTERFACE_ENTRYGROUP string = AVAHI_DBUS_NAME + ".EntryGroup"
const AVAHI_ENTRYGROUP_ADD_SERVICE string = AVAHI_INTERFACE_ENTRYGROUP + ".AddService"
const AVAHI_ENTRYGROUP_COMMIT = AVAHI_INTERFACE_ENTRYGROUP + ".Commit"

const AVAHI_IF_UNSPEC = int32(-1)
const PROTO_UNSPEC = int32(-1)
const PROTO_INET = int32(0)
const PROTO_INET6 = int32(1)

func registerServiceViaDBusAvahi(name, serviceType string, serverPort uint16, txt []string) bool {
	conn, err := dbus.SystemBus()

	if err != nil {
		fmt.Fprintln(os.Stderr, "Failed to connect to D-BUS session bus:", err)
		return false
	}

	obj := conn.Object(AVAHI_DBUS_NAME, "/")

	var groupPath dbus.ObjectPath
	call := obj.Call("org.freedesktop.Avahi.Server.EntryGroupNew", 0)
	if call.Err != nil {
		fmt.Fprintln(os.Stderr, "Avahi not found on D-BUS")
		return false
	}
	call.Store(&groupPath)

	group := conn.Object(AVAHI_DBUS_NAME, groupPath)

	var txtBytes [][]byte
	for _, s := range txt {
		txtBytes = append(txtBytes, []byte(s))
	}

	var flags = uint32(0)
	var domain = string("")
	var host = string("")
	group.Call(AVAHI_ENTRYGROUP_ADD_SERVICE, 0, AVAHI_IF_UNSPEC, PROTO_UNSPEC, flags, name, serviceType, domain, host, serverPort, txtBytes)
	group.Call(AVAHI_ENTRYGROUP_COMMIT, 0)

	return true
}

func registerService(serverPort uint16) bool {
	hostname, err := os.Hostname()
	if err != nil {
		log.Fatalln("Failed to fetch hostname")
	}

	txt := []string{"host=" + hostname}
	registered := registerServiceViaDBusAvahi("Reveller", "_reveller._tcp", serverPort, txt)
	if !registered {
		_, err := bonjour.Register("Reveller", "_reveller._tcp", "local", int(serverPort), txt, nil)
		if err != nil {
			return false
		}
	}

	return true
}
