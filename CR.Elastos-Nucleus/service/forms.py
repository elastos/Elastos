from django import forms
from django.forms import HiddenInput

from .models import UploadFile , SavedFileInformation
from .choices import *


class GenerateAPIKeyForm(forms.Form):
    did = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(GenerateAPIKeyForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()


class UploadAndSignForm(forms.ModelForm):
    network = forms.ChoiceField(
        choices=NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    file_content = forms.CharField(widget=forms.TextInput, required=False)
    private_key = forms.CharField(max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(UploadAndSignForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

        self.fields['file_content'].widget.attrs['rows'] = 10
        self.fields['file_content'].widget.attrs['cols'] = 100
        self.fields['private_key'].widget.attrs['rows'] = 1
        self.fields['private_key'].widget.attrs['cols'] = 100
        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 100

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class VerifyAndShowForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    Files = forms.ModelChoiceField(queryset=None)
    message_hash = forms.CharField(
        max_length=300, widget=forms.Textarea, help_text="Enter the Message Hash from DID")
    public_key = forms.CharField(
        max_length=300, widget=forms.Textarea, help_text="Enter the Public Key from DID")
    signature = forms.CharField(
        max_length=300, widget=forms.Textarea, help_text="Enter the Signature from DID")
    file_hash = forms.CharField(
        max_length=300, widget=forms.Textarea, help_text="Enter your File Hash output from HIVE")
    private_key = forms.CharField(
        max_length=300, widget=forms.Textarea, help_text="Enter your Private Key")
    api_key = forms.CharField(
        max_length=64, widget=forms.Textarea, help_text="Enter your API Key")

    def __init__(self, *args, **kwargs):
        did = kwargs.pop('did' , None)
        super(VerifyAndShowForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['message_hash'].widget.attrs['rows'] = 1
        self.fields['message_hash'].widget.attrs['cols'] = 80

        self.fields['public_key'].widget.attrs['rows'] = 1
        self.fields['public_key'].widget.attrs['cols'] = 80

        self.fields['signature'].widget.attrs['rows'] = 1
        self.fields['signature'].widget.attrs['cols'] = 80

        self.fields['file_hash'].widget.attrs['rows'] = 1
        self.fields['file_hash'].widget.attrs['cols'] = 80

        self.fields['private_key'].widget.attrs['rows'] = 1
        self.fields['private_key'].widget.attrs['cols'] = 80

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['Files'] = forms.ModelChoiceField(queryset=SavedFileInformation.objects.filter(did = did))


class CreateWalletForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    api_key = forms.CharField(max_length=300, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(CreateWalletForm, self).__init__(*args, **kwargs)
        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 100


class ViewWalletForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    chain = forms.CharField(max_length=300)
    address = forms.CharField(max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(ViewWalletForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['chain'].required = False

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 100

        self.fields['address'].widget.attrs['rows'] = 1
        self.fields['address'].widget.attrs['cols'] = 100

        self.fields['chain'].widget = HiddenInput()


class RequestELAForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    chain = forms.CharField(max_length=300)
    address = forms.CharField(max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(RequestELAForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['chain'].required = False

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 100

        self.fields['address'].widget.attrs['rows'] = 1
        self.fields['address'].widget.attrs['cols'] = 100

        self.fields['chain'].widget = HiddenInput()


class DeployETHContractForm(forms.ModelForm):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    eth_account_address = forms.CharField(
        max_length=300, widget=forms.Textarea)
    eth_private_key = forms.CharField(max_length=300, widget=forms.Textarea)
    eth_gas = forms.IntegerField()
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(DeployETHContractForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['did'].required = False

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 100

        self.fields['eth_account_address'].widget.attrs['rows'] = 1
        self.fields['eth_account_address'].widget.attrs['cols'] = 100

        self.fields['eth_private_key'].widget.attrs['rows'] = 1
        self.fields['eth_private_key'].widget.attrs['cols'] = 100

        self.fields['did'].widget = HiddenInput()

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class WatchETHContractForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    contract_address = forms.CharField(max_length=300, widget=forms.Textarea)
    contract_name = forms.CharField(max_length=300, widget=forms.Textarea)
    contract_code_hash = forms.CharField(max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(WatchETHContractForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 100

        self.fields['contract_address'].widget.attrs['rows'] = 1
        self.fields['contract_address'].widget.attrs['cols'] = 100

        self.fields['contract_name'].widget.attrs['rows'] = 1
        self.fields['contract_name'].widget.attrs['cols'] = 100

        self.fields['contract_code_hash'].widget.attrs['rows'] = 1
        self.fields['contract_code_hash'].widget.attrs['cols'] = 100


