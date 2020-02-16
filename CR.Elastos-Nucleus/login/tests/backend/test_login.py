from django.test import TestCase
from login.models import DIDUser

class DIDLoginCase(TestCase):

    def setUp(self):
        self.did = "testdid"
        self.DIDUser = DIDUser.objects.create(email="test@test.com", name="test_user", did=self.did)
        super(DIDLoginCase, self).tearDown()

    def tearDown(self):
        self.DIDUser.delete()
        super(DIDLoginCase, self).tearDown()

    def test_user_save(self):
        try:
            self.DIDUser.save()
            assert True
        except Exception as e:
            assert False

    def test_user_get(self):
        try:
            test_did = DIDUser.objects.get(did=self.did)
            assert True
        except Exception as e:
            assert False






