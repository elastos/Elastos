from django import forms
from django.forms import HiddenInput

from .models import UploadFile


class UploadAndSignForm(forms.ModelForm):
    private_key = forms.CharField(max_length=400)
    api_key = forms.CharField(max_length=300)

    def __init__(self, *args, **kwargs):
        super(UploadAndSignForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class VerifyAndShowForm(forms.Form):
    message_hash = forms.CharField(max_length=300, help_text="Enter the Message Hash from DID")
    public_key = forms.CharField(max_length=300, help_text="Enter the Public Key from DID")
    signature = forms.CharField(max_length=300, help_text="Enter the Signature from DID")
    file_hash = forms.CharField(max_length=300, help_text="Enter your File Hash output from HIVE")
    private_key = forms.CharField(max_length=300, help_text="Enter your Private Key")
    api_key = forms.CharField(max_length=300, help_text="Enter your API Key")


class DeployETHContractForm(forms.ModelForm):
    eth_account_address = forms.CharField(max_length=400)
    eth_account_password = forms.CharField(max_length=400)
    api_key = forms.CharField(max_length=300)

    def __init__(self, *args, **kwargs):
        super(DeployETHContractForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']