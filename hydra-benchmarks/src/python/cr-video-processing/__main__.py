import os
import stat
import subprocess
import requests

def call_ffmpeg(args):
    ret = subprocess.run([os.path.join("/tmp", "ffmpeg"), '-y'] + args,
            stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE, 
            stderr=subprocess.STDOUT
    )

    if ret.returncode != 0:
        print('Invocation of ffmpeg failed!')
        print('Out: ', ret.stdout.decode('utf-8'))
        raise RuntimeError()

def to_gif(video, duration):
    output = 'processed-{}.gif'.format(os.path.basename(video))
    call_ffmpeg(["-i", video,
        "-t",
        "{0}".format(duration),
        "-vf",
        "fps=10,scale=320:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse",
        "-loop", "0",
        output])
    return output

def set_ffmpeg_executable():
    try:
        os.chmod("/tmp/ffmpeg", os.stat("/tmp/ffmpeg").st_mode | stat.S_IEXEC)
    except OSError:
        pass

def videoprocessing(ffmpeg_url, video_url):
    if not os.path.exists("/tmp/ffmpeg"):
        with open("/tmp/ffmpeg", 'wb') as ofile:
            response = requests.get(ffmpeg_url)
            ofile.write(response.content)
        set_ffmpeg_executable()

    with open("video.mp4", 'wb') as ofile:
        response = requests.get(video_url)
        ofile.write(response.content)

    return to_gif("video.mp4", "1")

def main(args):
    try:
        return {"result": videoprocessing(args['ffmpeg_url'], args['video_url'])}
    except Exception as e:
        return {"result": str(e)}

#print(main({"ffmpeg_url": "http://localhost:8000/ffmpeg", "video_url": "http://127.0.0.1:8000/file_example_MP4_480_1_5MG.mp4"}))
