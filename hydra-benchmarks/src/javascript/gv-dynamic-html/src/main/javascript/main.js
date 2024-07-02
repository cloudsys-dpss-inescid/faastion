function random(b, e) {
    return Math.round(Math.random() * (e - b) + b);
}

const main = async function (args) {
    const output = new Map();
    const args_split = args.split(";");
    let url = args_split[0];
    let username = args_split[1];
    let nsize = args_split[2];
    var random_numbers = new Array(nsize);
    for(var i = 0; i < nsize; ++i) {
        random_numbers[i] = random(0, 100);
    }
    let template_args = "{ \"cur_time\": \"" + new Date().toLocaleString() + "\", \"username\" : \"" + username  + "\", \"random_numbers\": [" + random_numbers + "]}";
    output.set("result", polyHostAccess.mustache(String.fromCharCode(...polyHostAccess.downloadBytes(url)), template_args));
    return output;
}
