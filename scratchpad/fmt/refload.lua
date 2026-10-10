-- refload.lua LIST OUTDIR : EZ2REWRITE decrypt + decode, dump in ftload's format
package.path = (os.getenv("EZ2REWRITE") or "../EZ2REWRITE") .. "/?.lua;" .. package.path
local cipher = require("src.chart.cipher")
local ez = require("src.chart.ez")
local T = cipher.loadTable((os.getenv("EZ2REWRITE") or "../EZ2REWRITE") .. "/bible/data/cipher-tables/ez.bin")
local function u32(s, o) return s:byte(o+1) + s:byte(o+2)*256 + s:byte(o+3)*65536 + s:byte(o+4)*16777216 end
local ffi = require("ffi")
local fc = ffi.new("union { uint8_t b[4]; float f; }")
local function f32(s, o) fc.b[0], fc.b[1], fc.b[2], fc.b[3] = s:byte(o+1, o+4); return tonumber(fc.f) end
local i = 0
for line in io.lines(arg[1]) do
	local f = io.open(line, "rb"); local data = cipher.decrypt(f:read("*a"), T); f:close()
	local o = io.open(arg[2] .. "/" .. i .. ".ref", "w")
	-- pre-screen: a lane whose size field runs past the end of the file is
	-- damaged data the original would also overrun on; flag it, skip it
	local corrupt = false
	if data:sub(1, 4) == "EZFF" then
		local lanes = data:byte(0x8D) + data:byte(0x8E) * 256
		local pos = 0x96
		for l = 0, math.min(lanes, 96) - 1 do
			if pos + 0x4E > #data then break end
			local size = u32(data, pos + 0x4A)
			if pos + 0x4E + size > #data then corrupt = true; break end
			pos = pos + 0x4E + size
		end
	end
	if corrupt then
		o:write("SKIPPED\n"); o:close()
		local sk = io.open(arg[2] .. "/skip.txt", "a"); sk:write(i .. "\t" .. line .. "\n"); sk:close()
		i = i + 1
		goto continue
	end
	local ok, d = pcall(ez.decode, data)
	local base = line:match("([^/]+)$"):gsub("%.[^.]*$", "")
	local ver = data:byte(5) * 256 + data:byte(6)
	if not ok or (ver ~= 7 and ver ~= 8) or (ver == 8 and d.internal_name:lower() ~= base:lower()) then
		o:write("REJECTED\n")
	else
		local lanes = math.min(d.track_count, 96)
		o:write(string.format("H %d %.6g %.6g %d name=%s\n", d.ticks_per_measure, d.init_bpm, d.other_bpm, lanes, d.internal_name))
		local pos, total = 0x96, 0
		for l = 0, lanes - 1 do
			local t = d.tracks[l + 1]
			local size = u32(data, pos + 0x4A)
			local count = math.floor(size / 13)
			total = total + count
			-- rebuildIndex: f44 = the tick of the lane's LAST record (any type), 0 if empty
			local last = 0
			if count > 0 then last = u32(data, pos + 0x4E + (count - 1) * 13) end
			o:write(string.format("L %d ticks=%d count=%d name=%s\n", l, last, count, t and t.name or "?"))
			if count > 0x800 then o:write("OVERFLOW " .. count .. "\n") end
			local ni = 1
			for k = 0, math.min(count, 0x800) - 1 do
				local rec = pos + 0x4E + k * 13
				local ty = data:byte(rec + 5)
				if ty == 1 then
					local n = t.notes[ni]; ni = ni + 1
					o:write(string.format("N %d %d %d %d %d %d\n", n.tick, n.key, n.velocity, n.pan, n.note_type, n.length))
				elseif ty == 3 then
					o:write(string.format("B %d %.6g\n", u32(data, rec), f32(data, rec + 5)))
				end
			end
			pos = pos + 0x4E + size
		end
		o:write(string.format("M merged=%d lanesum=%d sorted=1\n", total, total))
	end
	o:close()
	i = i + 1
	::continue::
end
print(i .. " charts")
