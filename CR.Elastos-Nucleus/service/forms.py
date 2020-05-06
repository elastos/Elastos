from django import forms
from django.forms import HiddenInput

from .models import UploadFile, SavedFileInformation , SavedETHContractInformation
from .choices import *


GMUNET_BUTTON = 'width: 150px; height: 44px; border-radius: 4px; background-color: #5ac8fa;'


class GenerateAPIKeyForm(forms.Form):
    did = forms.CharField(max_length=64)

    def __init__(self, *args, **kwargs):
        super(GenerateAPIKeyForm, self).__init__(*args, **kwargs)
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

class UploadAndSignForm(forms.ModelForm):
    network = forms.ChoiceField(
        choices=NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    file_content = forms.CharField(widget=forms.Textarea, required=False)
    file_name = forms.CharField(required=True, widget=forms.Textarea)
    private_key = forms.CharField(max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(UploadAndSignForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

        self.fields['file_content'].widget.attrs['rows'] = 5
        self.fields['file_content'].widget.attrs['cols'] = 100
        self.fields['file_name'].widget.attrs['rows'] = 1
        self.fields['file_name'].widget.attrs['cols'] = 100
        self.fields['private_key'].widget.attrs['rows'] = 1
        self.fields['private_key'].widget.attrs['cols'] = 85
        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON
        self.fields['uploaded_file'].label = ""

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class VerifyAndShowForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK_GMU, label="", initial='', widget=forms.Select(), required=True)
    select_files = forms.ModelChoiceField(queryset=None)
    message_hash = forms.CharField(
        max_length=300, widget=forms.Textarea)
    public_key = forms.CharField(
        max_length=300, widget=forms.Textarea)
    signature = forms.CharField(
        max_length=300, widget=forms.Textarea)
    file_hash = forms.CharField(
        max_length=300, widget=forms.Textarea)
    private_key = forms.CharField(
        max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(
        max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        did = kwargs.pop('did', None)
        super(VerifyAndShowForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""

        self.fields['message_hash'].label = "Message Hash(Enter the Message Hash from DID)"
        self.fields['message_hash'].widget.attrs['rows'] = 1
        self.fields['message_hash'].widget.attrs['cols'] = 115

        self.fields['public_key'].label = "Enter the Public Key from DID"
        self.fields['public_key'].widget.attrs['rows'] = 1
        self.fields['public_key'].widget.attrs['cols'] = 85

        self.fields['signature'].label = "Enter the Signature from DID"
        self.fields['signature'].widget.attrs['rows'] = 1
        self.fields['signature'].widget.attrs['cols'] = 160

        self.fields['file_hash'].label = "Enter your File Hash output from HIVE"
        self.fields['file_hash'].widget.attrs['rows'] = 1
        self.fields['file_hash'].widget.attrs['cols'] = 58

        self.fields['private_key'].label = "Enter your Private Key"
        self.fields['private_key'].widget.attrs['rows'] = 1
        self.fields['private_key'].widget.attrs['cols'] = 85

        self.fields['api_key'].label = "Enter your API Key"
        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['select_files'].label = "Select a previously uploaded file"
        self.fields['select_files'] = forms.ModelChoiceField(queryset=SavedFileInformation.objects.filter(did=did), required=False)

        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON


class CreateWalletForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    api_key = forms.CharField(max_length=300, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(CreateWalletForm, self).__init__(*args, **kwargs)
        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON


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
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['address'].widget.attrs['rows'] = 1
        self.fields['address'].widget.attrs['cols'] = 55

        self.fields['chain'].widget = HiddenInput()

        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON


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
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['address'].widget.attrs['rows'] = 1
        self.fields['address'].widget.attrs['cols'] = 55

        self.fields['chain'].widget = HiddenInput()

        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON


class DeployETHContractForm(forms.ModelForm):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    file_name = forms.CharField(required=True, widget=forms.Textarea)
    eth_account_address = forms.CharField(
        max_length=300, widget=forms.Textarea)
    eth_private_key = forms.CharField(max_length=300, widget=forms.Textarea)
    eth_gas = forms.IntegerField(widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        super(DeployETHContractForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['did'].required = False
        self.fields['did'].widget = HiddenInput()

        self.fields['file_name'].widget.attrs['rows'] = 1
        self.fields['file_name'].widget.attrs['cols'] = 55

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['eth_account_address'].widget.attrs['rows'] = 1
        self.fields['eth_account_address'].widget.attrs['cols'] = 55

        self.fields['eth_private_key'].widget.attrs['rows'] = 1
        self.fields['eth_private_key'].widget.attrs['cols'] = 85

        self.fields['eth_gas'].widget.attrs['rows'] = 1
        self.fields['eth_gas'].widget.attrs['cols'] = 16


        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON
        self.fields['uploaded_file'].label = ""

    class Meta:
        model = UploadFile
        fields = ['did', 'uploaded_file']


class WatchETHContractForm(forms.Form):
    network = forms.ChoiceField(
        choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
    select_file = forms.ModelChoiceField(queryset=None)
    contract_address = forms.CharField(max_length=300, widget=forms.Textarea)
    contract_name = forms.CharField(max_length=300, widget=forms.Textarea)
    contract_code_hash = forms.CharField(max_length=300, widget=forms.Textarea)
    api_key = forms.CharField(max_length=64, widget=forms.Textarea)

    def __init__(self, *args, **kwargs):
        did = kwargs.pop('did' , None)
        super(WatchETHContractForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""

        self.fields['api_key'].widget.attrs['rows'] = 1
        self.fields['api_key'].widget.attrs['cols'] = 80

        self.fields['contract_address'].widget.attrs['rows'] = 1
        self.fields['contract_address'].widget.attrs['cols'] = 55

        self.fields['contract_name'].widget.attrs['rows'] = 1
        self.fields['contract_name'].widget.attrs['cols'] = 24

        self.fields['contract_code_hash'].widget.attrs['rows'] = 1
        self.fields['contract_code_hash'].widget.attrs['cols'] = 55

        self.fields['network'].widget.attrs['style'] = GMUNET_BUTTON

        self.fields['select_file'].label = "Select a previously uploaded contract"
        self.fields['select_file'] = forms.ModelChoiceField(queryset=SavedETHContractInformation.objects.filter(did=did) ,required=False)


class SuggestServiceForm(forms.Form):

    service_category = forms.ChoiceField(choices=CATEGORY, label="Category", initial='', widget=forms.Select(),
                                         required=True)
    service_name = forms.CharField(max_length=200, widget=forms.Textarea)
    service_description = forms.CharField(max_length=1000, widget=forms.Textarea, label="Description(Please be as "
                                                                                        "specific as possible)")
    service_reasoning = forms.CharField(max_length=3000, widget=forms.Textarea, label="Why do you think this service "
                                                                                      "would be useful?")

    def __init__(self, *args, **kwargs):
        super(SuggestServiceForm, self).__init__(*args, **kwargs)

        self.fields['service_name'].widget.attrs['rows'] = 1
        self.fields['service_name'].widget.attrs['cols'] = 50

        self.fields['service_description'].widget.attrs['rows'] = 5
        self.fields['service_description'].widget.attrs['cols'] = 50

        self.fields['service_reasoning'].widget.attrs['rows'] = 5
        self.fields['service_reasoning'].widget.attrs['cols'] = 50
