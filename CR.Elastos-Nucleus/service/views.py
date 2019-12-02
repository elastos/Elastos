import json
import sys
import requests
from decouple import config
from . import forms
from . import models
from django.http import HttpResponse
from django.shortcuts import redirect, render
sys.path.append(config('ADENINE_STUBS'))
sys.path.append(config('ADENINE'))
from adenine.common import Common
from adenine.console import Console

def generate_key(request):
    if(request.method == 'POST'):
        common = Common()
        userDID = request.session['did']
        getData =  common.generate_api_request(config('SHARED_SECRET_ADENINE'), userDID)
        if(getData.status == True):
            API_KEY = getData.api_key
            context = {
                'API_KEY': API_KEY,
            }
            sent_json = json.dumps({'API_KEY': API_KEY})
            return HttpResponse(sent_json, content_type='application/JSON')
        else:
            print("Didnt get a response")
    else:
        return render(request, "Services/generateKey.html")


def upload_and_sign(request):
    if(request.method == 'POST'):
        models.files.objects.all().delete()
        privateKey = request.POST['privateKey']
        apiKey = request.POST['apiKey']
        name = request.POST['name']
        form = forms.UploadFile(request.POST, request.FILES)
        if(form.is_valid()):
            form.save()
            files = models.files.objects.get(name=name)
            file_path = files.uploaded_file.path
            console = Console()
            getData = console.upload_and_sign(apiKey , config('PRIVATE_KEY') , file_path) #for development purposes only the config needs to be users private key
            jsonData = json.loads(getData.output)
            if(getData.status==True):
                 models.files.objects.get(name=name).delete()
                 msg = jsonData['result']['msg']
                 pub = jsonData['result']['pub']
                 sig = jsonData['result']['sig']
                 Hash = jsonData['result']['hash']
                 output = True
                 return render(request, "Services/uploadAndSign.html", {"msg": msg , "pub":pub, "sig": sig, "Hash":Hash, 'output': output})
            else:
                return HttpResponse("grpc call didnt work")
        else:
            return HttpResponse("Didnt work")
    else:
        output = False
        form = forms.UploadFile()
        return render(request, "Services/uploadAndSign.html" , {'form': form , 'output':output})

def verify_and_show(request):
    if request.method == 'POST':
        output = True
        msg = request.POST['Message']
        privatekey = config('PRIVATE_KEY') #for development purposes , for deployment change it to the users private key
        publicKey = request.POST['Public_Key']
        Hash = request.POST['Hash']
        sig = request.POST['Signature']
        api = request.POST['API_Key']
        request_input = {
            "msg": msg,
            "pub": publicKey,
            "sig": sig,
            "hash": Hash,
            "private_key": privatekey
        }
        console = Console()
        getData = console.verify_and_show(api , request_input)
        if(getData.status == True):
            content = getData.output
            return render(request , 'Services/verifyAndShow.html' , {'output':output , 'content' : content})
        else:
            return HttpResponse("File didnt Exist")
    else:
        output = False
        form = forms.verifyAndShow()
        return render(request , 'Services/verifyAndShow.html' , {'output':output  , 'form':form})

def request_ela(request):
    return render(request, "Services/requestELA.html")

def vote_supernodes(request):
    return render(request, "Services/voteSupernodes.html")

def run_contract(request):
    return render(request, "Services/runContract.html")

def save_did_data(request):
    return render(request, "Services/saveData.html")

def retrieve_did_data(request):
    return render(request, "Services/retrieveData.html")
