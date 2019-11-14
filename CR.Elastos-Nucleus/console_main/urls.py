"""elastos_console_main URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/2.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include
from django.conf.urls import url
from qr_code import urls as qr_code_urls
from . import views

urlpatterns = [
    url('admin/', admin.site.urls),
    url(r'^at/', include('admin_tools.urls')),
    url(r'^$', views.index, name="index"),
    url(r'^service/', include('service.urls'), name='service'),
    url(r'^login/', include('login.urls'), name='login'),
    url(r'^qr_code/', include(qr_code_urls, namespace='qr_code')),
]