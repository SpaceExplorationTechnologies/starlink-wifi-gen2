-- Copyright 2009 Daniel Dickinson
-- Licensed to the public under the Apache License 2.0.

module("luci.controller.luci_diag.luci_diag_devinfo", package.seeall)

function index()
   local e

   e = entry({"admin", "voice", "diag", "phones"}, arcombine(cbi("luci_diag/smap_devinfo"), cbi("luci_diag/smap_devinfo_config")), _("Phones"), 10)
   e.leaf = true
   e.subindex = true
   e.dependent = true

   e = entry({"admin", "voice", "diag", "phones", "config"}, cbi("luci_diag/smap_devinfo_config"), _("Configure"), 10)

   e = entry({"admin", "status", "smap_devinfo"}, cbi("luci_diag/smap_devinfo"), _("SIP Devices on Network"), 120)
   e.leaf = true
   e.dependent = true

   e = entry({"admin", "network", "diag_config", "netdiscover_devinfo_config"}, cbi("luci_diag/netdiscover_devinfo_config"), _("Network Device Scan"), 100)
   e.leaf = true
   e.dependent = true

   e = entry({"admin", "network", "diag_config", "smap_devinfo_config"}, cbi("luci_diag/smap_devinfo_config"), _("SIP Device Scan"))
   e.leaf = true
   e.dependent = true

   e = entry({"admin", "status", "netdiscover_devinfo"}, cbi("luci_diag/netdiscover_devinfo"), _("Devices on Network"), 90)
   e.dependent = true

   e = entry({"admin", "network", "mactodevinfo"}, cbi("luci_diag/mactodevinfo"), _("MAC Device Info Overrides"), 190)
   e.dependent = true

   e = entry({"mini", "diag", "phone_scan"}, cbi("luci_diag/smap_devinfo_mini"), _("Phone Scan"), 100)
   e.dependent = true

   e = entry({"mini", "voice", "phones", "phone_scan_config"}, cbi("luci_diag/smap_devinfo_config_mini"), _("Config Phone Scan"), 90)
   e.dependent = true

   e = entry({"mini", "diag", "netdiscover_devinfo"}, cbi("luci_diag/netdiscover_devinfo_mini"), _("Network Device Scan"), 10)
   e.dependent = true

   e = entry({"mini", "network", "netdiscover_devinfo_config"}, cbi("luci_diag/netdiscover_devinfo_config_mini"), _("Device Scan Config"))
   e.dependent = true

end
