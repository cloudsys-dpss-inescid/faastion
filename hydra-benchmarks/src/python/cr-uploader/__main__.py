import urllib.request
import requests

def uploader(url):
    with urllib.request.urlopen(url) as response:
        return requests.post(url, headers={'Content-Type': 'image/png'}, data=response.read())

def main(args):
    try:
        return { "result": uploader(args['url']).status_code }
    except Exception as e:
        return {"result": str(e)}
