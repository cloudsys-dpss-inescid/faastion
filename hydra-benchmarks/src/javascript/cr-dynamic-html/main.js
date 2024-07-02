const mustache = require('mustache')
const http = require('http');
const url = require('url');

function random(b, e) {
    return Math.round(Math.random() * (e - b) + b);
}

function get_request(template_url) {
    return new Promise((resolve, reject) => {
        let q = url.parse(template_url, true);
        var options = {
            host: q.hostname,
            path: q.path,
            port: q.port
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
    let username = args['username'];
    let nsize = args['nsize'];
    var random_numbers = new Array(nsize);
    for(var i = 0; i < nsize; ++i) {
        random_numbers[i] = random(0, 100);
    }
    let template_args = { "cur_time": new Date().toLocaleString(), "username" : username, "random_numbers": random_numbers };
    let template = await get_request(args['url']);
    return { "result": mustache.render(template, template_args) } 
}

exports.main = main;

/*
(async function () {
    console.log(await main({"url": "http://localhost:8000/template.html", "username": "rbruno", "nsize": 10}));
})();
*/
