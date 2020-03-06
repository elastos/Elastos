from django import forms

from .choices import *


class SelectNetworkForm(forms.Form):
    network = forms.ChoiceField(choices=NETWORK, label="", initial='', widget=forms.Select(), required=True)
