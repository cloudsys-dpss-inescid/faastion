import datetime
import os
import requests
from squiggle import transform

def dna(fasta_url):
    with open("/tmp/bacillus_subtilis.fasta", 'wb') as ofile:
        response = requests.get(fasta_url)
        ofile.write(response.content)

    process_begin = datetime.datetime.now()
    result = transform(open("/tmp/bacillus_subtilis.fasta", "r").read())
    process_end = datetime.datetime.now()

    return {
            'result': (result[0][:10], result[1][:10]),
            'measurement': {
                'compute_time': (process_end - process_begin) / datetime.timedelta(microseconds=1)
            }
    }

def main(fasta_url):
    try:
        return {"result": dna(fasta_url)}
    except Exception as e:
        return {"result": str(e)}

#main("http://127.0.0.1:8000/bacillus_subtilis.fasta")
