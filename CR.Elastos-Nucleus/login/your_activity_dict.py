activity_dict = {
    'generate_key': 'UserServiceSessionVars',
    'upload_and_sign': 'SavedFileInformation',
    'verify_and_show': 'SavedFileInformation',
    'create_wallet': 'UserServiceSessionVars',
    'view_wallet': 'UserServiceSessionVars',
    'request_ela': 'UserServiceSessionVars',
    'deploy_eth_contract': 'UserServiceSessionVars',
    'watch_eth_contract': 'UserServiceSessionVars'
}


def get_activity_model(view_name):
    if view_name in activity_dict:
        return activity_dict[view_name]
    else:
        return None
