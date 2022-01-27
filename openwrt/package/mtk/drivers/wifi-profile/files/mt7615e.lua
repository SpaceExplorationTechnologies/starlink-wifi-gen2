#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path

local vif_prefix = {"ra", "rai", "rae", "rax", "ray", "raz",
    "apcli", "apclix", "apclii", "apcliy", "apclie", "apcliz", }

local function esc(x)
   return (x:gsub('%%', '%%%%')
            :gsub('^%^', '%%^')
            :gsub('%$$', '%%$')
            :gsub('%(', '%%(')
            :gsub('%)', '%%)')
            :gsub('%.', '%%.')
            :gsub('%[', '%%[')
            :gsub('%]', '%%]')
            :gsub('%*', '%%*')
            :gsub('%+', '%%+')
            :gsub('%-', '%%-')
            :gsub('%?', '%%?'))
end

function add_vif_into_lan(vif)
    local mtkwifi = require("mtkwifi")
    local brvifs = mtkwifi.__trim( mtkwifi.read_pipe("uci get network.lan.ifname"))

    if not string.match(brvifs, esc(vif)) then
        nixio.syslog("debug", "add "..vif.." into lan")
        -- brvifs = brvifs.." "..vif
        -- os.execute("uci set network.lan.ifname=\""..brvifs.."\"") --netifd will down vif form /etc/config/network
        -- os.execute("uci commit")
        -- os.execute("ubus call network.interface.lan add_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
        os.execute("ubus -t 5 wait_for network.interface.lan") -- sometimes br-lan is not ready yet
        os.execute("brctl addif br-lan "..vif) -- double insurance for rare failure
	if mtkwifi.exists("/proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6") then
            os.execute("echo 1 > /proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6")
	end
    else
        nixio.syslog("debug", vif.." is already added into lan")
    end
    brvifs = string.split(mtkwifi.__trim((mtkwifi.read_pipe("ls /sys/class/net/br-lan/brif/"))))
    for _,vif in ipairs(brvifs) do
        nixio.syslog("debug", "brvif = "..vif)
    end
end

function del_vif_from_lan(vif)
    local mtkwifi = require("mtkwifi")
    local brvifs = mtkwifi.__trim(mtkwifi.read_pipe("uci get network.lan.ifname"))
    if string.match(brvifs, esc(vif)) then
        -- brvifs = mtkwifi.__trim(string.gsub(brvifs, esc(vif), ""))
        nixio.syslog("debug", "del "..vif.." from lan")
        -- os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
        -- os.execute("uci commit")
        -- os.execute("ubus call network.interface.lan remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
        if mtkwifi.exists("/proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6") then
            os.execute("echo 0 > /proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6")
        end
        os.execute("brctl delif br-lan "..vif)
    end
end

function mt7615e_up(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local wifi_services_exist = false
    if  mtkwifi.exists("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mt7615e_up called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mt7615e_up: dev "..devname.." not found!")
            return
        end
        local profile = mtkwifi.search_dev_and_profile()[devname]
        local cfgs = mtkwifi.load_profile(profile)
        -- we have to bring up main_ifname first, main_ifname will create all other vifs.
        if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
            nixio.syslog("debug", "mt7615e_up: ifconfig "..dev.main_ifname.." up")
            os.execute("ifconfig "..dev.main_ifname.." up")
            add_vif_into_lan(dev.main_ifname)
            if wifi_services_exist then
                -- SpaceX: Don't check miniupnpd is up
                -- miniupnpd_chk(devname, dev.main_ifname, true)
            end
        else
            nixio.syslog("err", "mt7615e_up: main_ifname "..dev.main_ifname.." missing, quit!")
            return
        end
        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif ~= dev.main_ifname and
            (  string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or (string.match(vif, esc(dev.apcli_ifname).."[0-9]+") and
                cfgs.ApCliEnable ~= "0" and cfgs.ApCliEnable ~= "")
            or (string.match(vif, esc(dev.wds_ifname).."[0-9]+") and
                cfgs.WdsEnable ~= "0" and cfgs.WdsEnable ~= "")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+"))
            then
                nixio.syslog("debug", "mt7615e_up: ifconfig "..vif.." up")
                os.execute("ifconfig "..vif.." up")
                add_vif_into_lan(vif)
                if wifi_services_exist and string.match(vif, esc(dev.ext_ifname).."[0-9]+") then
                    -- SpaceX: Don't check miniupnpd is up
                    -- miniupnpd_chk(devname, vif, true)
                end
            -- else nixio.syslog("debug", "mt7615e_up: skip "..vif..", prefix not match "..pre)
            end
        end
        os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=1")
        if wifi_services_exist then
             d8021xd_chk(devname, dev.ext_ifname, dev.main_ifname, true)
        end
    elseif mtkwifi.exists("/etc/wireless/mt7615e/"..devname..".dat") then
        for _, pre in ipairs(vif_prefix) do
            -- we have to bring up root vif first, root vif will create all other vifs.
            if mtkwifi.exists("/sys/class/net/"..pre.."0") then
                nixio.syslog("debug", "mt7615e_up: ifconfig "..pre.."0 up")
                os.execute("ifconfig "..pre.."0 up")
                add_vif_into_lan(pre.."0")
                if wifi_services_exist then
                    -- SpaceX: Don't check miniupnpd is up
                    -- miniupnpd_chk(devname, pre.."0", true)
                end
            end

            for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
            do
                -- nixio.syslog("debug", "mt7615e_up: navigate "..pre)
                if string.match(vif, pre.."[1-9]+") then
                    nixio.syslog("debug", "mt7615e_up: ifconfig "..vif.." up")
                    os.execute("ifconfig "..vif.." up")
                    add_vif_into_lan(vif)
                    if wifi_services_exist then
                        -- SpaceX: Don't check miniupnpd is up
                        -- miniupnpd_chk(devname, vif, true)
                    end
                -- else nixio.syslog("debug", "mt7615e_up: skip "..vif..", prefix not match "..pre)
                end
            end
            if wifi_services_exist  then
                d8021xd_chk(devname,pre,pre.."0",true)
            end
        end
    else nixio.syslog("debug", "mt7615e_up: skip "..devname..", config not exist")
    end

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
        os.execute("/etc/init.d/man start")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mt7615e*.need_reload")
end

function mt7615e_down(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local wifi_services_exist = false
    if  mtkwifi.exists("/lib/wifi/wifi_services.lua") then
        wifi_services_exist = require("wifi_services")
    end

    nixio.syslog("debug", "mt7615e_down called!")

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
    end

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mt7615e_down: dev "..devname.." not found!")
            return
        end
        if not mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
            nixio.syslog("err", "mt7615e_down: main_ifname "..dev.main_ifname.." missing, quit!")
            return
        end
        os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=0")
        if wifi_services_exist then
            d8021xd_chk(devname,dev.ext_ifname,dev.main_ifname)
        end
        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif == dev.main_ifname
            or string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or string.match(vif, esc(dev.apcli_ifname).."[0-9]+")
            or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+")
            then
                if wifi_services_exist then
                    if vif == dev.main_ifname
                    or string.match(vif, esc(dev.ext_ifname).."[0-9]+") then
                        -- SpaceX: Don't check miniupnpd is up
                        -- miniupnpd_chk(devname, vif)
                    end
                end
                nixio.syslog("debug", "mt7615e_down: ifconfig "..vif.." down")
                os.execute("ifconfig "..vif.." down")
                del_vif_from_lan(vif)
            -- else nixio.syslog("debug", "mt7615e_down: skip "..vif..", prefix not match "..pre)
            end
        end
    elseif mtkwifi.exists("/etc/wireless/mt7615e/"..devname..".dat") then
        for _, pre in ipairs(vif_prefix) do
            if wifi_services_exist then
                d8021xd_chk(devname,pre,pre.."0")
            end
            for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
            do
                if string.match(vif, pre.."[0-9]+") then
                    nixio.syslog("debug", "mt7615e_down: ifconfig "..vif.."down")
                    os.execute("ifconfig "..vif.." down")
                    del_vif_from_lan(vif)
                    if wifi_services_exist then
                        -- SpaceX
                        -- miniupnpd_chk(devname, vif)
                    end
                -- else nixio.syslog("debug", "mt7615e_down: skip "..vif..", prefix not match "..pre)
                end
            end
        end
    else nixio.syslog("debug", "mt7615e_down: skip "..devname..", config not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mt7615e*.need_reload")
end

function mt7615e_reload(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local normal_reload = true
    local qsetting = false
    local path, profiles
    local devs, l1parser = mtkwifi.__get_l1dat()
    nixio.syslog("debug", "mt7615e_reload called!")

    if mtkwifi.exists("/lib/wifi/quick_setting.lua") then
        qsetting = true
        profiles = mtkwifi.search_dev_and_profile()
    end

    -- For one card , all interface should be down, then up
    if not devname then
        for devname, dev in pairs(devs.devname_ridx) do
                mt7615e_down(devname)
        end
        for devname, dev in mtkwifi.__spairs(devs.devname_ridx) do
            if qsetting then
                -- Create devname.last for quick setting
                path = profiles[devname]
                if not mtkwifi.exists("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".applied") then
                    os.execute("cp -f "..path.." "..mtkwifi.__profile_previous_settings_path(path))
                else
                    os.execute("cp -f "..mtkwifi.__profile_applied_settings_path(path)..
                        " "..mtkwifi.__profile_previous_settings_path(path))
                end
            end
            mt7615e_up(devname)
        end
    else
        if qsetting then
            path = profiles[devname]
            normal_reload = quick_settings(devname, path)
        end

        if normal_reload then
            local dev = devs.devname_ridx[devname]
            assert(mtkwifi.exists(dev.init_script))
            local compatname = dev.init_compatible
            for devname, dev in pairs(devs.devname_ridx) do
                if dev.init_compatible == compatname then
                    mt7615e_down(devname)
                end
            end
         for devname, dev in mtkwifi.__spairs(devs.devname_ridx) do
                if dev.init_compatible == compatname then
                    mt7615e_up(devname)
                end
            end
        else mt7615e_up(devname) end
    end
end

function mt7615e_restart(devname)
    local nixio = require("nixio")
    local uci  = require "luci.model.uci".cursor()
    local mtkwifi = require("mtkwifi")
    local devs, l1parser = mtkwifi.__get_l1dat()
    local hwnat_en = uci:get("hwnat","global","enabled")
    nixio.syslog("debug", "mt7615e_restart called!")

    -- if wifi driver is built-in, it's necessary action to reboot the device
    if mtkwifi.exists("/sys/module/mt_wifi") == false then
        os.execute("echo reboot_required > /tmp/mtk/wifi/reboot_required")
        return
    end

    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mt7615e_down(devname)
            end
        end
    else
         for devname, dev in pairs(devs.devname_ridx) do
             mt7615e_down(devname)
         end
    end
    os.execute("rmmod mt_whnat")
    os.execute("/etc/init.d/fwdd stop")
    os.execute("rmmod mtfwd")
    os.execute("rmmod mt_wifi")
    os.execute("rmmod mtkhnat")
    if hwnat_en then
        os.execute("rmmod hw_nat")
        os.execute("insmod /lib/modules/ralink/hw_nat.ko")
    end
    os.execute("modprobe mtkhnat")
    os.execute("modprobe mt_wifi")
    os.execute("modprobe mtfwd")
    os.execute("/etc/init.d/fwdd start")
    os.execute("modprobe mt_whnat")
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mt7615e_up(devname)
            end
        end
    else
        for devname, dev in pairs(devs.devname_ridx) do
            mt7615e_up(devname)
        end
    end
end

function mt7615e_reset(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615e_reset called!")
    if mtkwifi.exists("/rom/etc/wireless/mediatek/") then
        os.execute("rm -rf /etc/wireless/mediatek/")
        os.execute("cp -rf /rom/etc/wireless/mediatek/ /etc/wireless/")
        mt7615e_reload(devname)
    else
        nixio.syslog("debug", "mt7615e_reset: /rom"..profile.." missing, unable to reset!")
    end
end

function mt7615e_status(devname)
    return wifi_common_status()
end

function mt7615e_detect(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mt7615e_detect called!")

    for _,dev in ipairs(mtkwifi.get_all_devs()) do
        local relname = string.format("%s%d%d",dev.maindev,dev.mainidx,dev.subidx)
        print([[
config wifi-device ]]..relname.."\n"..[[
    option type mt7615e
    option vendor ralink
]])
        for _,vif in ipairs(dev.vifs) do
            print([[
config wifi-iface
    option device ]]..relname.."\n"..[[
    option ifname ]]..vif.vifname.."\n"..[[
    option network lan
    option mode ap
    option ssid ]]..vif.__ssid.."\n")
        end
    end
end
