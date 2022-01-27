-- Copyright 2012 Manuel Munz <freifunk at somakoma de>
-- Licensed to the public under the Apache License 2.0.

local require = require
module "luci.controller.freifunk.widgets"


function index()

	local page  = node("admin", "freifunk", "widgets")
	page.target = cbi("freifunk/widgets/widgets_overview")
	page.title  = _("Widgets")
	page.i18n   = "widgets"
	page.order  = 30

	local page  = node("admin", "freifunk", "widgets", "widget")
	page.target = cbi("freifunk/widgets/widget")
	page.leaf  = true

	local page  = node("freifunk", "search_redirect")
	page.target = call("search_redirect")
	page.leaf  = true
end

function search_redirect()
	local dsp = require "luci.dispatcher"
	local http = require "luci.http"
	local engine = http.formvalue("engine")
	local searchterms = http.formvalue("searchterms") or ""
	if engine then
		http.redirect(engine .. searchterms)
	else
		http.redirect(dsp.build_url())
	end
end
