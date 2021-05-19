from django.contrib.sessions.middleware import SessionMiddleware
from django.test import TestCase, RequestFactory
from django.test import client
from decouple import config
from console_main.views import check_ela_auth, landing, get_elastos_sign_in_url, get_elaphant_sign_in_url, \
    did_callback_elastos , privacy_policy_pdf , terms_conditions_pdf
import jwt
import secrets
import json
import urllib.parse
from console_main.tests.test_helper import get_elastos_fields, get_elephant_url_fields, get_elastos_jwt_token


class GeneralViewTests(TestCase):
    def setUp(self) -> None:
        self.client = client.Client()
        self.requests = RequestFactory()
        self.random_num = secrets.randbelow(9999999999)

    def test_privacy_policy(self):
        request = self.requests.request()
        response =  privacy_policy_pdf(request)
        assert response.status_code != 404

    def test_terms_and_conditions(self):
        request = self.requests.request()
        response = terms_conditions_pdf(request)
        assert response.status_code != 404

    def test_landing(self):
        request = self.requests.request()
        middleware = SessionMiddleware()
        middleware.process_request(request)
        request.session.save()
        random = self.random_num
        request.session['elaState'] = random
        response = landing(request)
        assert response.status_code == 200

    def tearDown(self) -> None:
        self.client = None
        self.requests = None
        self.random_num = 0


class LoginTests(TestCase):

    def setUp(self) -> None:
        self.client = client.Client()
        self.requests = RequestFactory()
        self.random_num = secrets.randbelow(999999999999)

    def test_login_required(self):
        response = self.client.get('/service/generate_key', follow=True)
        assert '/' in response.redirect_chain[0]

    def test_get_elastos_url(self):
        request = self.requests.request()
        response = get_elastos_sign_in_url(request, self.random_num)
        response_list = response.split('/')
        decoded_token = jwt.decode(response_list[3], verify=False)
        for i in get_elastos_fields():
            if i not in decoded_token:
                assert False, 'elastos url is missing {0} field'.format(i)

        callback_url = config('DIDLOGIN_APP_URL') + '/did_callback_elastos'

        if callback_url != decoded_token['callbackurl']:
            assert False, 'callback_url is wrong'

        if decoded_token['claims'].get('name') is not True or decoded_token['claims'].get('email') is not True:
            assert False, 'claims field boolean value is missing or False'

        assert True

    def test_did_elephant_url(self):
        request = self.requests.request()
        response = get_elaphant_sign_in_url(request, self.random_num)
        response_list = response.split('elaphant://identity?')
        decoded = urllib.parse.parse_qs(response_list[1])
        string_keys = decoded.keys()
        for i in get_elephant_url_fields():
            if i not in string_keys:
                assert False, 'elephant url is missing {0} field '.format(i)

        callback_url = config('DIDLOGIN_APP_URL') + '/did_callback_elaphant'

        if callback_url != decoded['CallbackUrl'][0]:
            assert False, 'callback_url is missing or wrong'
        assert True

    def test_did_elastos_callback(self):
        landing_request = self.requests.request()
        middleware = SessionMiddleware()
        middleware.process_request(landing_request)
        landing_request.session.save()
        random = self.random_num
        landing_request.session['elaState'] = random
        response = landing(landing_request)
        if response.status_code != 200:
            assert False, 'landing status code: {0}'.format(response.status_code)
        jwt_token = get_elastos_jwt_token(self.random_num)
        data = {'jwt': jwt_token}
        data = json.dumps(data)
        callback_response = self.requests.post('/did_callback_elastos', data, content_type='application/json')
        response = did_callback_elastos(callback_response)
        if response.status_code == 200:
            assert True
        else:
            assert False, 'did_callback_elastos status code:{0}'.format(response.status_code)

    def test_check_ela_auth_false(self):
        request = self.requests.request()
        middleware = SessionMiddleware()
        middleware.process_request(request)
        request.session.save()
        response = check_ela_auth(request)
        assert response.status_code == 403

    def test_check_ela_auth(self):
        landing_request = self.requests.request()
        middleware = SessionMiddleware()
        middleware.process_request(landing_request)
        landing_request.session.save()
        random = self.random_num
        landing_request.session['elaState'] = random  # set so that this number can be used to identify jwt_tokens
        response = landing(landing_request)
        if response.status_code != 200:
            assert False, 'landing status code: {0}'.format(response.status_code)
        jwt_token = get_elastos_jwt_token(self.random_num)
        data = {'jwt': jwt_token}
        data = json.dumps(data)
        callback_response = self.requests.post('/did_callback_elastos', data, content_type='application/json')
        response = did_callback_elastos(callback_response)
        if response.status_code != 200:
            assert False, 'did_callback_elastos status code:{0}'.format(response.status_code)

        response = check_ela_auth(landing_request)
        if response.status_code == 200:
            assert True
        else:
            assert False , 'check_ela_auth status code:{0}'.format(response.status_code)

    def tearDown(self) -> None:
        self.client = None
        self.requests = None
        self.random_num = 0
