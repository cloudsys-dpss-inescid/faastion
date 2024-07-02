const main = async function (url) {
    const output = new Map()
    polyHostAccess.uploadBytes(url, polyHostAccess.downloadBytes(url));
    output.set("result", url)
    return output
}
