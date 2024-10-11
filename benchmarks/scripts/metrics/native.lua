local socket = require("socket")

local function_name = os.getenv("function_name")

request = function()

    wrk.headers["Content-Type"] = "application/json"
    wrk.body = '{"name":"' .. function_name .. '","async":"false","cached":"true","arguments":""}'
    
    return wrk.format("POST", nil, nil, wrk.body)
end