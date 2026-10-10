-- refstr.lua LIST OUTDIR : EZ2REWRITE's parseStr over a list, dumped in ftstr's format
local R = os.getenv("EZ2REWRITE") or "../EZ2REWRITE"
package.path = R .. "/?.lua;" .. R .. "/?/init.lua;" .. package.path
local bga = require("src.bga")
local function u32(s, o) return s:byte(o+1) + s:byte(o+2)*256 + s:byte(o+3)*65536 + s:byte(o+4)*16777216 end
local i = 0
for line in io.lines(arg[1]) do
	local f = io.open(line, "rb"); local data = f:read("*a"); f:close()
	local o = io.open(arg[2] .. "/" .. i .. ".ref", "w")
	local p = bga.parseStr(data)
	if not p then
		o:write("FAIL\n")
	else
		o:write(string.format("H fps=%d frames=%d layers=%d\n", u32(data, 8), u32(data, 12), #p.layers))
		local off = 36
		for l, L in ipairs(p.layers) do
			o:write(string.format("L %d textures=%d keys=%d\n", l - 1, #L.textures, #L.keyframes))
			for _, t in ipairs(L.textures) do o:write("T " .. t .. "\n") end
			off = off + 4 + #L.textures * 128 + 4
			for k = 0, #L.keyframes - 1 do
				local w = {}
				for b = 0, 0x7c - 4, 4 do w[#w + 1] = string.format("%08x", u32(data, off + k * 124 + b)) end
				o:write("K " .. table.concat(w) .. "\n")
			end
			off = off + #L.keyframes * 124
		end
	end
	o:close()
	i = i + 1
end
print(i .. " files")
