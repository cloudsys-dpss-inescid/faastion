import urllib.request
import requests

def uploader(url):
    with urllib.request.urlopen(url) as response:
        return requests.post(url, headers={'Content-Type': 'image/png'}, data=response.read())

def main(url):
    try:
        return {"result": uploader(url)}
    except Exception as e:
        return {"result": str(e)}
