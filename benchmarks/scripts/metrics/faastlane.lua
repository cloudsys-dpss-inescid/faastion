local socket = require("socket")

local function_name = os.getenv("function_name")
local thread_counter = 0
local max_workers = 15

function setup(thread)
    local id = (thread_counter % max_workers) + 1
    thread:set("thread_id", id)
    thread_counter = thread_counter + 1
end

request = function()
    local thread_id = wrk.thread:get("thread_id")

    wrk.headers["Content-Type"] = "application/json"
    wrk.body = '{"name":"' .. function_name .. thread_id .. '","async":"false","cached":"true","arguments":""}'
    
    return wrk.format("POST", nil, nil, wrk.body)
end

-- function response(status, headers, body)
--     print(body)
-- end

-- wrk.response = response