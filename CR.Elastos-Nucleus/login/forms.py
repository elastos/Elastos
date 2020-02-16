from django import forms
from django.utils.safestring import mark_safe
from django.contrib.auth.forms import UserCreationForm, UserChangeForm
from django.forms.widgets import HiddenInput

from .models import DIDUser


class DIDUserCreationForm(UserCreationForm):
    checkbox = forms.BooleanField(help_text=mark_safe("By clicking the checkbox, you acknowledge that you accept our "
                                                      "<a style=\"color: green\" data-toggle=\"modal\" "
                                                      "data-target=\"#privacyPolicy\">Privacy Policy</a> and "
                                                      "<a style=\"color: green\" data-toggle=\"modal\" "
                                                      "data-target=\"#termsAndConditions\">Terms of Conditions</a>."), required=True)

    def __init__(self, *args, **kwargs):
        super(DIDUserCreationForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['password1'].required = False
        self.fields['password2'].required = False
        self.fields['password1'].widget = HiddenInput()
        self.fields['password2'].widget = HiddenInput()
        self.fields['did'].widget.attrs['readonly'] = True
        self.fields['did'].widget.attrs['style'] = 'color: grey; background-color: lightgrey'
        self.fields['did'].label = "DID"
        self.fields['name'].label = "Nickname"
        self.fields['checkbox'].label = ""

    class Meta:
        model = DIDUser
        fields = ('email', 'name', 'did',)
        widgets = {
            'email': forms.Textarea(attrs={'rows': 1, 'cols': 64}),
            'name': forms.Textarea(attrs={'rows': 1, 'cols': 64}),
            'did': forms.Textarea(attrs={'rows': 1, 'cols': 64}),
        }

    def save(self, commit=True):
        user = super(DIDUserCreationForm, self).save(commit=False)
        user.email = self.cleaned_data['email']
        user.name = self.cleaned_data['name']
        user.did = self.cleaned_data['did']
        if commit:
            user.save()
        return user


class DIDUserChangeForm(UserChangeForm):

    def __init__(self, *args, **kwargs):
        super(DIDUserChangeForm, self).__init__(*args, **kwargs)
        self.label_suffix = ""
        self.fields['password'].required = False
        self.fields['password'].widget = HiddenInput()
        self.fields['did'].widget.attrs['readonly'] = True
        self.fields['did'].widget.attrs['style'] = 'color: grey; background-color: lightgrey'

    class Meta:
        model = DIDUser
        fields = ('email', 'name', 'did',)
        widgets = {
            'email': forms.Textarea(attrs={'rows': 1, 'cols': 64}),
            'name': forms.Textarea(attrs={'rows': 1, 'cols': 64}),
            'did': forms.Textarea(attrs={'rows': 1, 'cols': 64}),
        }
