local socket = require("socket")

local function_name = os.getenv("function_name")
local thread_counter = 0

function setup(thread)
    thread_counter = thread_counter + 1
    thread:set("thread_id", thread_counter)
end

request = function()
    local thread_id = wrk.thread:get("thread_id")

    wrk.headers["Content-Type"] = "application/json"
    wrk.body = '{"name":"' .. function_name .. thread_id .. '","async":"false","cached":"true","arguments":""}'
    
    return wrk.format("POST", nil, nil, wrk.body)
end