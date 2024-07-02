import datetime, os, uuid
import requests
from PIL import Image
import torch
from torchvision import transforms
from torchvision.models import resnet50

model = None

def classify(restnet_url, img_url):
    global model
    if not model:
        with open("/tmp/resnet50-19c8e357.pth", 'wb') as ofile:
            response = requests.get(restnet_url)
            ofile.write(response.content)

        model_process_begin = datetime.datetime.now()
        model = resnet50(pretrained=False)
        model.load_state_dict(torch.load("/tmp/resnet50-19c8e357.pth"))
        model.eval()
        model_process_end = datetime.datetime.now()
    else:
        model_process_begin = datetime.datetime.now()
        model_process_end = model_process_begin
   
    process_begin = datetime.datetime.now()
    with open("/tmp/snap.png", 'wb') as ofile:
        response = requests.get(img_url)
        ofile.write(response.content)

    input_image = Image.open("/tmp/snap.png").convert('RGB')
    preprocess = transforms.Compose([
        transforms.Resize(256),
        transforms.CenterCrop(224),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225]),
    ])
    input_tensor = preprocess(input_image)
    input_batch = input_tensor.unsqueeze(0) # create a mini-batch as expected by the model 
    output = model(input_batch)
    _, index = torch.max(output, 1)
    # The output has unnormalized scores. To get probabilities, you can run a softmax on it.
    prob = torch.nn.functional.softmax(output[0], dim=0)
    _, indices = torch.sort(output, descending = True)
    process_end = datetime.datetime.now()

    model_process_time = (model_process_end - model_process_begin) / datetime.timedelta(microseconds=1)
    process_time = (process_end - process_begin) / datetime.timedelta(microseconds=1)
    return {
            'result': {'idx': index.item()},
            'measurement': {
                'compute_time': process_time + model_process_time,
                'model_time': model_process_time,
            }
        }

def main(args):
    try:
        return {"result": classify(args['restnet_url'], args['img_url'])}
    except Exception as e:
        return {"result": str(e)}

#print(main({"restnet_url": "http://localhost:8000/resnet50-19c8e357.pth", "img_url": "http://localhost:8000/snap.png"}))
