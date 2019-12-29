from django.contrib import admin

from .models import UploadFile, UserServiceSessionVars , TrackUserPageVists


class UploadFileAdmin(admin.ModelAdmin):
    model = UploadFile
    list_display = ('did', 'uploaded_file')


admin.site.register(UploadFile, UploadFileAdmin)
admin.site.register(TrackUserPageVists)


class UserServiceSessionVarsAdmin(admin.ModelAdmin):
    model = UserServiceSessionVars
    list_display = ('did', 'api_key', 'mnemonic_mainchain', 'private_key_mainchain', 'public_key_mainchain', 'address_mainchain', 'private_key_did', 'public_key_did', 'address_did', 'did_did', 'address_eth', 'private_key_eth')


admin.site.register(UserServiceSessionVars, UserServiceSessionVarsAdmin)
