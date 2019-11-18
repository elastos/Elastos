from django.contrib.auth.forms import UserCreationForm, UserChangeForm
from django.forms.widgets import HiddenInput

from .models import DIDUser


class DIDUserCreationForm(UserCreationForm):

    def __init__(self, *args, **kwargs):
        super(DIDUserCreationForm, self).__init__(*args, **kwargs)
        self.fields['password1'].required = False
        self.fields['password2'].required = False
        self.fields['password1'].widget = HiddenInput()
        self.fields['password2'].widget = HiddenInput()
        self.fields['did'].widget.attrs['readonly'] = True
        self.fields['did'].widget.attrs['style'] = 'color: grey; background-color: lightgrey'

    class Meta(UserCreationForm):
        model = DIDUser
        fields = ('email', 'name', 'did',)


class DIDUserChangeForm(UserChangeForm):
    class Meta:
        model = DIDUser
        fields = ('email',)
