from django.forms import ModelForm
from django import forms
from .models import files


class UploadFile(forms.ModelForm):
    privateKey = forms.CharField(max_length=400)
    apiKey = forms.CharField(max_length=300)
    class Meta:
        model = files
        fields = ['name', 'uploaded_file']


class verifyAndShow(forms.Form):
    Message = forms.CharField(max_length=300 , help_text="Enter the message Hash from DID")
    Public_Key = forms.CharField(max_length=300 , help_text="Enter the public Key from DID")
    Signature = forms.CharField(max_length=300 , help_text="Enter the signature from DID")
    Hash = forms.CharField(max_length=300 , help_text="Enter your Hash output from HIVE")
    Private_Key = forms.CharField(max_length=300 , help_text="Enter your private Key")
    API_Key = forms.CharField(max_length=300 , help_text="Enter your API Key")