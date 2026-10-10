-- refscr.lua LIST OUTDIR : EZ2REWRITE's parseScr over a list, dumped in ftscr's format
local R = os.getenv("EZ2REWRITE") or "../EZ2REWRITE"
package.path = R .. "/?.lua;" .. R .. "/?/init.lua;" .. package.path
local bga = require("src.bga")
local i = 0
for line in io.lines(arg[1]) do
	local f = io.open(line, "rb"); local data = f:read("*a"); f:close()
	local o = io.open(arg[2] .. "/" .. i .. ".ref", "w")
	if data:sub(1, 4) ~= "SCR0" then
		o:write("LEGACY\n")
	else
		local p = bga.parseScr(data)
		if not p then o:write("FAIL\n")
		else
			for c = 1, 10 do
				o:write("C " .. (c - 1) .. "\n")
				for _, e in ipairs(p.channels[c] or {}) do
					o:write(string.format("E %d %d %s\n", e.tick, e.type, e.name or ""))
				end
			end
		end
	end
	o:close()
	i = i + 1
end
print(i .. " files")
