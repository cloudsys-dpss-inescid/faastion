local function_names = {"nativehw"} 
local function_index = 1

local socket = require("socket")

function setup(thread) 
    thread:set("start_time", 0) 
    -- thread:set("name","")
end

request = function()
    local function_name = function_names[function_index]
    function_index = function_index + 1
    if function_index > #function_names then
        function_index = 1
    end

    wrk.headers["Content-Type"] = "application/json"
    wrk.body = '{"name":"' .. function_name .. '","async":"false","cached":"true","arguments":""}'
    
    wrk.thread:set("start_time", socket.gettime())
    -- wrk.thread:set("name",function_name)
    return wrk.format("POST", nil, nil, wrk.body)
end

function response(status, headers, body)
    local end_time = socket.gettime()
    local start_time = wrk.thread:get("start_time")
    -- local name = wrk.thread:get("name")
    local latency_milli = (end_time - start_time) * 1000  
    print(string.format("%.2f", latency_milli))
    -- print(body)
end

wrk.response = response
