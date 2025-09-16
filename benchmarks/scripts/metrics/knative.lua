local socket = require("socket")

local post_body = os.getenv("body")

request = function()

    wrk.headers["Content-Type"] = "application/json"
    wrk.body = post_body
    
    return wrk.format("POST", nil, nil, wrk.body)
end

-- function response(status, headers, body)
--     print(body)
-- end

-- wrk.response = response
