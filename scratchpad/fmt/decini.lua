package.path = (os.getenv("EZ2REWRITE") or "../EZ2REWRITE") .. "/?.lua;" .. package.path
local c = require("src.chart.cipher")
local T = c.loadTable((os.getenv("EZ2REWRITE") or "../EZ2REWRITE") .. "/bible/data/cipher-tables/ini.bin")
local f = io.open(arg[1], "rb"); io.write(c.decrypt(f:read("*a"), T)); f:close()
