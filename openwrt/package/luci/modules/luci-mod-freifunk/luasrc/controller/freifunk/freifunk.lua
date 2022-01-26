-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.freifunk.freifunk", package.seeall)

function index()
	local uci = require "luci.model.uci".cursor()
	local page

	-- Frontend
	page          = node()
	page.lock     = true
	page.target   = alias("freifunk")
	page.subindex = true
	page.index    = false

	page          = node("freifunk")
	page.title    = _("Freifunk")
	page.target   = alias("freifunk", "index")
	page.order    = 5
	page.setuser  = "nobody"
	page.setgroup = "nogroup"
	page.i18n     = "freifunk"
	page.index    = true

	page          = node("freifunk", "index")
	page.target   = template("freifunk/index")
	page.title    = _("Overview")
	page.order    = 10
	page.indexignore = true

	page          = node("freifunk", "contact")
	page.target   = template("freifunk/contact")
	page.title    = _("Contact")
	page.order    = 15

	page          = node("freifunk", "status")
	page.target   = template("freifunk/public_status")
	page.title    = _("Status")
	page.order    = 20
	page.i18n     = "base"
	page.setuser  = false
	page.setgroup = false

	entry({"freifunk", "status.json"}, call("jsonstatus"))
	entry({"freifunk", "status", "zeroes"}, call("zeroes"), "Testdownload")

	if nixio.fs.access("/usr/sbin/luci-splash") then
		assign({"freifunk", "status", "splash"}, {"splash", "publicstatus"}, _("Splash"), 40)
	end

	page = assign({"freifunk", "olsr"}, {"admin", "status", "olsr"}, _("OLSR"), 30)
	page.setuser = false
	page.setgroup = false

	if nixio.fs.access("/etc/config/luci_statistics") then
		assign({"freifunk", "graph"}, {"admin", "statistics", "graph"}, _("Statistics"), 40)
	end

	-- backend
	assign({"mini", "freifunk"}, {"admin", "freifunk"}, _("Freifunk"), 5)
	entry({"admin", "freifunk"}, alias("admin", "freifunk", "index"), _("Freifunk"), 5)

	page        = node("admin", "freifunk")
	page.target = template("freifunk/adminindex")
	page.title  = _("Freifunk")
	page.order  = 5

	page        = node("admin", "freifunk", "basics")
	page.target = cbi("freifunk/basics")
	page.title  = _("Basic Settings")
	page.order  = 5
	
	page        = node("admin", "freifunk", "basics", "profile")
	page.target = cbi("freifunk/profile")
	page.title  = _("Profile")
	page.order  = 10

	page        = node("admin", "freifunk", "basics", "profile_expert")
	page.target = cbi("freifunk/profile_expert")
	page.title  = _("Profile (Expert)")
	page.order  = 20

	page        = node("admin", "freifunk", "Index-Page")
	page.target = cbi("freifunk/user_index")
	page.title  = _("Index Page")
	page.order  = 50

	page        = node("admin", "freifunk", "contact")
	page.target = cbi("freifunk/contact")
	page.title  = _("Contact")
	page.order  = 15

	entry({"freifunk", "map"}, template("freifunk-map/frame"), _("Map"), 50)
	entry({"freifunk", "map", "content"}, template("freifunk-map/map"), nil, 51)
	entry({"admin", "freifunk", "profile_error"}, template("freifunk/profile_error"))
end

function zeroes()
	local string = require "string"
	local http = require "luci.http"
	local zeroes = string.rep(string.char(0), 8192)
	local cnt = 0
	local lim = 1024 * 1024 * 1024
	
	http.prepare_content("application/x-many-zeroes")

	while cnt < lim do
		http.write(zeroes)
		cnt = cnt + #zeroes
	end
end

function jsonstatus()
	local root = {}
	local sys = require "luci.sys"
	local uci = require "luci.model.uci"
	local util = require "luci.util"
	local http = require "luci.http"
	local json = require "luci.json"
	local ltn12 = require "luci.ltn12"
	local version = require "luci.version"
	local webadmin = require "luci.tools.webadmin"

	local cursor = uci.cursor_state()

	local ffzone = webadmin.firewall_find_zone("freifunk")
	local ffznet = ffzone and cursor:get("firewall", ffzone, "network")
	local ffwifs = ffznet and util.split(ffznet, " ") or {}

	local sysinfo = util.ubus("system", "info") or { }
	local boardinfo = util.ubus("system", "board") or { }

	local loads = sysinfo.load or { 0, 0, 0 }

	local memory = sysinfo.memory or {
		total = 0,
		free = 0,
		shared = 0,
		buffered = 0
	}

	local swap = sysinfo.swap or {
		total = 0,
		free = 0
	}


	root.protocol = 1

	root.system = {
		uptime = { sysinfo.uptime or 0 },
		loadavg = { loads[1] / 65535.0, loads[2] / 65535.0, loads[3] / 65535.0 },
		sysinfo = {
			boardinfo.system or "?",
			boardinfo.model or "?",
			memory.total,
			0, -- former cached memory
			memory.buffered,
			memory.free,
			0, -- former bogomips
			swap.total,
			0, -- former cached swap
			swap.free
		},
		hostname = boardinfo.hostname
	}

	root.firmware = {
		luciname=version.luciname,
		luciversion=version.luciversion,
		distname=version.distname,
		distversion=version.distversion
	}

	root.freifunk = {}
	cursor:foreach("freifunk", "public", function(s)
		root.freifunk[s[".name"]] = s
	end)

	cursor:foreach("system", "system", function(s)
		root.geo = {
			latitude = s.latitude,
			longitude = s.longitude
		}
	end)

	root.network = {}
	root.wireless = {devices = {}, interfaces = {}, status = {}}
	local wifs = root.wireless.interfaces
	local netdata = luci.sys.net.deviceinfo() or {}

	for _, vif in ipairs(ffwifs) do
		root.network[vif] = cursor:get_all("network", vif)
		root.wireless.devices[vif] = cursor:get_all("wireless", vif)
		cursor:foreach("wireless", "wifi-iface", function(s)
			if s.device == vif and s.network == vif then
				wifs[#wifs+1] = s
				if s.ifname then
					local iwinfo = luci.sys.wifi.getiwinfo(s.ifname)
					if iwinfo then
						root.wireless.status[s.ifname] = { }

						local _, f
						for _, f in ipairs({
							"channel", "txpower", "bitrate", "signal", "noise",
							"quality", "quality_max", "mode", "ssid", "bssid", "encryption", "ifname"
						}) do
							root.wireless.status[s.ifname][f] = iwinfo[f]
						end
					end
				end
			end
		end)
	end

	http.prepare_content("application/json")
	ltn12.pump.all(json.Encoder(root):source(), http.write)
end
