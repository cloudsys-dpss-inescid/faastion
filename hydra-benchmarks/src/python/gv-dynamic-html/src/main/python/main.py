import requests
from datetime import datetime                                                   
from random import sample 

def dynamic_html(url, username, nsize):
    response = requests.get(url)
    template_args = "{ \"cur_time\": \"" + str(datetime.now()) + "\", \"username\" : \"" + username  + "\", \"random_numbers\": " + str(sample(range(0, 1000), int(nsize))) + "}"
    template = response.text
    return polyHostAccess.mustache(template, template_args)

def main(args):
    try:
        url, username, nsize = args.split(";")
        return {"result": dynamic_html(url, username, nsize)}
    except Exception as e:
        return {"result": str(e)}
