from flask import Flask, request

app = Flask(__name__)

@app.post("/upload")
def upload():
    file = request.files["file"]
    path = "/tmp/uploads/" + file.filename
    file.save(path)
    return {"path": path}, 200

app.run(host="0.0.0.0", port=9696)
