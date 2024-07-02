const http = require('http');
const url = require('url');

function post_request(post_url, data) {
    return new Promise((resolve, reject) => {
        let q = url.parse(post_url, true);
        var options = {
            host: q.hostname,
            path: q.path,
            port: q.port,
            method: 'POST',
        };
        let ret = http.request(options, function(response) {
            response.on('data', d => {
                resolve(response.statusCode);
            });
            response.on('error', (error) => {
                reject(error);
            });
        });
        ret.end();
    });
}

function get_request(get_url) {
    return new Promise((resolve, reject) => {
        let q = url.parse(get_url, true);
        var options = {
            host: q.hostname,
            path: q.path,
            port: q.port,
            method: 'GET',
        };
        http.get(options, function(response) {
            response.on('data', d => {
                resolve(d.toString('utf8'));
            });
            response.on('error', (error) => {
                reject(error);
            });
        });
    });
}

async function main(args) {
    try {
        let get_data = await get_request(args['url']);
        let post_data = await post_request(args['url'], get_data);
        return { "result": post_data } 
    }
    catch(err) {
        return { "result": err } 
    }
}

exports.main = main;

/*
(async function () {
    console.log(await main({"url": "http://127.0.0.1:8000/snap.png"}));
})();
*/
