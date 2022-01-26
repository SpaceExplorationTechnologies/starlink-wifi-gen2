-- Copyright 2017 Dirk Brenken (dev@brenken.org)
-- This is free software, licensed under the Apache License, Version 2.0

local fs = require("nixio.fs")
local util = require("luci.util")
local uci = require("uci")
local adbinput = uci.get("adblock", "global", "adb_whitelist") or "/etc/adblock/adblock.whitelist"

if not nixio.fs.access(adbinput) then
	m = SimpleForm("error", nil, translate("Input file not found, please check your configuration."))
	return m
end

m = SimpleForm("input", nil)
m:append(Template("adblock/config_css"))
m.reset = false

s = m:section(SimpleSection, nil,
	translatef("This form allows you to modify the content of the adblock whitelist (%s).<br />", adbinput)
	.. translate("Please add only one domain per line. Comments introduced with '#' are allowed - ip addresses, wildcards and regex are not."))

f = s:option(TextValue, "data")
f.datatype = "string"
f.rows = 20
f.rmempty = true

function f.cfgvalue()
	return nixio.fs.readfile(adbinput) or ""
end

function f.write(self, section, data)
	return nixio.fs.writefile(adbinput, "\n" .. util.trim(data:gsub("\r\n", "\n")) .. "\n")
end

function s.handle(self, state, data)
	return true
end

return m
