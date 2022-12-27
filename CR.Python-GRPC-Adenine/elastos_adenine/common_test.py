import pytest
import json
from decouple import Config, RepositoryEnv, config

from elastos_adenine.stubs import health_check_pb2
from elastos_adenine.health_check import HealthCheck
from elastos_adenine.common import Common

test_input = {}
env_config = None


@pytest.fixture(scope="module")
def set_test_input(request):
    global env_config

    DOTENV_FILE = '.env.test'
    env_config = Config(RepositoryEnv(DOTENV_FILE))

    test_input['host'] = config('GRPC_SERVER_HOST')
    test_input['port'] = config('GRPC_SERVER_PORT')
    test_input['production'] = config('PRODUCTION', default=False, cast=bool)
    test_input['network'] = env_config('NETWORK')
    test_input['ela_to_use'] = env_config('ELA_TO_USE')
    test_input['ela_eth_to_use'] = env_config('ELA_ETH_TO_USE')
    test_input['did_to_use'] = env_config('DID_TO_USE')
    test_input['private_key_to_use'] = env_config('PRIVATE_KEY_TO_USE')
    return test_input


@pytest.mark.dependency()
def test_health_check(set_test_input):
    # Health Check
    try:
        health_check = HealthCheck(test_input['host'], test_input['port'], test_input['production'])
        response = health_check.check()
        assert response.status == health_check_pb2.HealthCheckResponse.SERVING, "grpc server is not running properly"
    except Exception as e:
        health_check_status = False
        assert health_check_status == True, f"grpc server is not running properly: {e}"


@pytest.mark.dependency(depends=["test_health_check"])
def test_generate_api_key():
    # Generate API Key
    common = Common(test_input['host'], test_input['port'], test_input['production'])
    response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), test_input['did_to_use'])
    if response['status']:
        json_output = json.loads(response['output'])
        for i in json_output['result']:
            if i == 'api_key':
                api_key_to_use = json_output['result'][i]
    assert response['status'] == True, "In Generate Api Key-> " + response['status_message']
    assert len(api_key_to_use) == 64, "Testing API Key length Failed"


@pytest.mark.dependency(depends=["test_health_check"])
def test_get_api_key():
    # Get API Key
    common = Common(test_input['host'], test_input['port'], test_input['production'])
    response = common.get_api_key_request(config('SHARED_SECRET_ADENINE'), test_input['did_to_use'])
    if response['status']:
        json_output = json.loads(response['output'])
        for i in json_output['result']:
            if i == 'api_key':
                api_key_to_use = json_output['result'][i]
    assert response['status'] == True, "In Get Api Key-> " + response['status_message']
    assert len(api_key_to_use) == 64, "Testing API Key length Failed"
