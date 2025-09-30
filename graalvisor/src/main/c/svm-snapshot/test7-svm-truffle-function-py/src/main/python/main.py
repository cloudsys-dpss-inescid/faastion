import shutil
import random

def compression(path):
    return shutil.make_archive(path + str(random.randint(1, 100)), 'zip', path)

def main(args):
    try:
        return {"result": compression(args)}
    except Exception as e:
        return {"result": str(e)}
