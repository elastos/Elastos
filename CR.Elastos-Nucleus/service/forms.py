from django import forms
from django.forms import HiddenInput

from .models import UploadFile
from .choices import * 


class GenerateAPIKeyForm(forms.Form):
    did = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(GenerateAPIKeyForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()


class GenerateAPIKeyForm(forms.Form):
    did = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(GenerateAPIKeyForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()


class UploadAndSignForm(forms.ModelForm):
    network = forms.ChoiceField(choices = NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    private_key = forms.CharField(max_length=300)
    api_key = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(UploadAndSignForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class VerifyAndShowForm(forms.Form):
    network = forms.ChoiceField(choices = NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    message_hash = forms.CharField(max_length=300, help_text="Enter the Message Hash from DID")
    public_key = forms.CharField(max_length=300, help_text="Enter the Public Key from DID")
    signature = forms.CharField(max_length=300, help_text="Enter the Signature from DID")
    file_hash = forms.CharField(max_length=300, help_text="Enter your File Hash output from HIVE")
    private_key = forms.CharField(max_length=300, help_text="Enter your Private Key")
    api_key = forms.CharField(max_length=64, help_text="Enter your API Key")


class CreateWalletForm(forms.Form):
    network = forms.ChoiceField(choices = NETWORK, label="", initial='', widget=forms.Select(), required=True)
    api_key = forms.CharField(max_length=300)


class ViewWalletForm(forms.Form):
    network = forms.ChoiceField(choices = NETWORK, label="", initial='', widget=forms.Select(), required=True)
    chain = forms.CharField(max_length=300)
    address = forms.CharField(max_length=300)
    api_key = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(ViewWalletForm, self).__init__(*args, **kwargs)
        self.fields['chain'].required = False
        self.fields['chain'].widget = HiddenInput()


class RequestELAForm(forms.Form):
    network = forms.ChoiceField(choices = NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    chain = forms.CharField(max_length=300)
    address = forms.CharField(max_length=300)
    api_key = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(RequestELAForm, self).__init__(*args, **kwargs)
        self.fields['chain'].required = False
        self.fields['chain'].widget = HiddenInput()


class DeployETHContractForm(forms.ModelForm):
    network = forms.ChoiceField(choices = NETWORK, label="", initial='', widget=forms.Select(), required=True)
    eth_account_address = forms.CharField(max_length=300)
    eth_private_key = forms.CharField(max_length=300)
    eth_gas = forms.IntegerField()
    api_key = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(DeployETHContractForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class WatchETHContractForm(forms.Form):
    network = forms.ChoiceField(choices = NETWORK, label="", initial='', widget=forms.Select(), required=True)
    contract_address = forms.CharField(max_length=300)
    contract_name = forms.CharField(max_length=300)
    contract_code_hash = forms.CharField(max_length=300)
    api_key = forms.CharField(max_length=64)
