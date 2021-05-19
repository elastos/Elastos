activity_dict = {
    'generate_key': 'UserServiceSessionVars',
    'upload_and_sign': 'SavedFileInformation',
    'verify_and_show': 'SavedFileInformation',
    'create_wallet': 'UserServiceSessionVars',
    'view_wallet': 'UserServiceSessionVars',
    'request_ela': 'UserServiceSessionVars',
    'deploy_eth_contract': 'SavedETHContractInformation',
    'watch_eth_contract': 'SavedETHContractInformation'
}


activity_time_intervals = (
    ('weeks', 604800),  # 60 * 60 * 24 * 7
    ('days', 86400),    # 60 * 60 * 24
    ('hours', 3600),    # 60 * 60
    ('minutes', 60),
    ('seconds', 1),
)


def display_time_elapsed(seconds, granularity=2):
    result = []

    for name, count in activity_time_intervals:
        value = seconds // count
        if value:
            seconds -= value * count
            if value == 1:
                name = name.rstrip('s')
            result.append("{} {}".format(value, name))
    return ', '.join(result[:granularity])


def get_activity_model(view_name):
    if view_name in activity_dict:
        return activity_dict[view_name]
    else:
        return None
