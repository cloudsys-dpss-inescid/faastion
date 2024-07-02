--local function_names = {"factors", "matmul", "httprequest", "nativehw", "sleep", "hw"}
local function_names = {"hw", "sleep", "httprequest", "factors"}
local function_index = 1

request = function()
    local function_name = function_names[function_index]
    function_index = function_index + 1
    if function_index > #function_names then
        function_index = 1
    end

    wrk.headers["Content-Type"] = "application/json"
    wrk.body = '{"name":"' .. function_name .. '","async":"false","cached":"true","arguments":""}'
    return wrk.format("POST", nil)
end

response = function(status, headers, body)
    print(body)
end

wrk.response = response