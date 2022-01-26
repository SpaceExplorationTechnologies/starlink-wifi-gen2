-- Long ago, this lua module lived with MTK router firmware in harmony.
-- Then, everything changed when Collin Valley attacked and gutted the lua module for security reasons.
-- Only MTK, the supplier of SpaceX chips, could stop him, but when the module needed them most, they vanished.
-- Many moons passed, and Michael Lu somehow became an intern at SpaceX, and although his programming skills are
-- questionable, he still managed to find this file before he could even mesh two Starlink V2 routers.
-- But I believe, Michael can save this lua script...

-- TODO(SATSW-24742) https://jira.spacex.corp/browse/SATSW-24742:
-- 1. Eventually, we should not rely on this Lua script to get the mesh runtime topology, map_helper.c should be
--    invoked from the Go code in payload.
-- 2. Topology should be read from the returned value of map_helper.c instead of being read from /tmp/dump.txt.

package.path = '/lib/wifi/?.lua;'..package.path
module("luci.controller.mtkwifi", package.seeall)

local onboardingType = 0;
local ioctl_help = require "ioctl_helper"
local map_help
if pcall(require, "map_helper") then
    map_help = require "map_helper"
end
local http = require("luci.http")
local mtkwifi = require("mtkwifi")

local logDisable = 10

function get_runtime_topology()
    local map_help

    if pcall(require, "map_helper") then
        map_help = require "map_helper"
    end
    if map_help then
        local r = c_get_runtime_topology()

        local t = read_topology_file("/tmp/dump.txt")
        print(t)
    end
end

function read_topology_file(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return content
end

get_runtime_topology()
