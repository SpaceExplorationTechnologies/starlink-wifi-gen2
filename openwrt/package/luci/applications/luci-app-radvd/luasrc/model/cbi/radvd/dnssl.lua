-- Copyright 2010 Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

local sid = arg[1]
local utl = require "luci.util"

m = Map("radvd", translatef("Radvd - DNSSL"),
	translate("Radvd is a router advertisement daemon for IPv6. " ..
		"It listens to router solicitations and sends router advertisements " ..
		"as described in RFC 4861."))

m.redirect = luci.dispatcher.build_url("admin/network/radvd")

if m.uci:get("radvd", sid) ~= "dnssl" then
	luci.http.redirect(m.redirect)
	return
end


s = m:section(NamedSection, sid, "interface", translate("DNSSL Configuration"))
s.addremove = false


--
-- General
--

o = s:option(Flag, "ignore", translate("Enable"))
o.rmempty = false

function o.cfgvalue(...)
	local v = Flag.cfgvalue(...)
	return v == "1" and "0" or "1"
end

function o.write(self, section, value)
	Flag.write(self, section, value == "1" and "0" or "1")
end


o = s:option(Value, "interface", translate("Interface"),
	translate("Specifies the logical interface name this section belongs to"))

o.template = "cbi/network_netlist"
o.nocreate = true
o.optional = false

function o.formvalue(...)
	return Value.formvalue(...) or "-"
end

function o.validate(self, value)
	if value == "-" then
		return nil, translate("Interface required")
	end
	return value
end

function o.write(self, section, value)
	m.uci:set("radvd", section, "ignore", 0)
	m.uci:set("radvd", section, "interface", value)
end


o = s:option(DynamicList, "suffix", translate("Suffix"),
	translate("Advertised Domain Suffixes"))

o.optional    = false
o.rmempty     = false
o.datatype    = "hostname"
function o.cfgvalue(self, section)
	local l = { }
	local v = m.uci:get_list("radvd", section, "suffix")
	for v in utl.imatch(v) do
		l[#l+1] = v
	end
	return l
end


o = s:option(Value, "AdvDNSSLLifetime", translate("Lifetime"),
	translate("Specifies the maximum duration how long the DNSSL entries are used for name resolution."))

o.datatype = 'or(uinteger,"infinity")'
o.placeholder = 1200


return m
