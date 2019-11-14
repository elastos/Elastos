from django.contrib import admin

# Register your models here.
from .models import DIDUser, DIDRequest


class DIDUserAdmin(admin.ModelAdmin):
    model = DIDUser
    list_display = ('id', 'name', 'email', 'did', 'created_at', 'updated_at')


admin.site.register(DIDUser, DIDUserAdmin)


class DIDRequestAdmin(admin.ModelAdmin):
    model = DIDRequest
    list_display = ('id', 'state', 'data', 'created_at', 'updated_at')


admin.site.register(DIDRequest, DIDRequestAdmin)
