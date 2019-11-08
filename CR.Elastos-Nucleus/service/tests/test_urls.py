from django.test import SimpleTestCase
from django.urls import reverse, resolve
from service.views import generate_key, file_upload


class testUrls(SimpleTestCase):
    def test_generate_key_url_is_resolved(self):
        url = reverse("service:generateKey")
        self.assertEquals(resolve(url).func, generate_key)

    def test_file_upload_url_is_resolved(self):
        url = reverse("service:fileUpload")
        self.assertEquals(resolve(url).func, file_upload)
