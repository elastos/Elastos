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
from django.urls import include
from django.urls import path
from django.conf.urls import url
from qr_code import urls as qr_code_urls
from . import views

app_name = "console_main"

urlpatterns = [
    url('admin/', admin.site.urls),
    url(r'^at/', include('admin_tools.urls')),
    url(r'^$', views.landing, name="landing"),
    path('check_ela_auth', views.check_ela_auth, name="check_ela_auth"),
    path('did_callback_elaphant', views.did_callback_elaphant, name="did_callback_elaphant"),
    path('did_callback_elastos', views.did_callback_elastos, name="did_callback_elastos"),
    url(r'^qr_code/', include(qr_code_urls, namespace='qr_code')),
    url(r'^privacy_policy_pdf', views.privacy_policy_pdf, name='privacy_policy_pdf'),
    url(r'^terms_conditions_pdf', views.terms_conditions_pdf, name='terms_conditions_pdf'),
    url(r'^login/', include('login.urls'), name='login'),
    url(r'^service/', include('service.urls'), name='service'),
    url(r'^browser/', include('browser.urls'), name='browser'),
    url(r'^elastos_trinity/', include('elastos_trinity.urls'), name='elastos_trinity'),
]
