const sharp = require('sharp');
const fs = require('fs');
const http = require('http');
const url = require('url');

function main(args) {
    let iname = "img-" + (Math.random()).toString().substring(10) + ".png";
    const file = fs.createWriteStream(iname);
    let q = url.parse(args['url'], true);
    var options = {
        host: q.hostname,
        path: q.path,
        port: q.port
    };
    http.get(options, function(response) {
        sharp_resizer = sharp().resize(100, 100).png();
        response.pipe(sharp_resizer).pipe(file);
        file.on("finish", () => {
            file.close();
        });
    });
    return { "result": iname } 
}

exports.main = main;
