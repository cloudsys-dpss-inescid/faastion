import requests
from datetime import datetime
from random import sample
import pystache

def dynamic_html(url, username, nsize):
    response = requests.get(url)
    template_args = { 'cur_time': str(datetime.now()), 'username' : username, 'random_numbers': sample(range(0, 1000), int(nsize)) }
    template = response.text
    return pystache.render(template, template_args)

def main(args):
    try:
        return {"result": dynamic_html(args['url'], args['username'], args['nsize'])}
    except Exception as e:
        return {"result": str(e)}
