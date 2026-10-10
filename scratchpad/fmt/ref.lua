-- ref.lua TABLE LIST OUTDIR : EZ2REWRITE's decrypt over a list, writes OUTDIR/<i>.ref
package.path = (os.getenv("EZ2REWRITE") or "../EZ2REWRITE") .. "/?.lua;" .. package.path
local cipher = require("src.chart.cipher")
local T = cipher.loadTable(arg[1])
local i = 0
for line in io.lines(arg[2]) do
	local f = io.open(line, "rb")
	if f then
		local d = f:read("*a"); f:close()
		local o = io.open(arg[3] .. "/" .. i .. ".ref", "wb")
		o:write(cipher.decrypt(d, T)); o:close()
	end
	i = i + 1
end
print(i .. " files")
