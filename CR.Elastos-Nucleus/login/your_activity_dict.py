activity_dict = {
        'generate_key': 'you just generated a new API Key',
        'upload_and_sign': 'you just uploaded the file:',#use .format(item) to show
        'verify_and_show': 'you just verified a new file ',
        'create_wallet': 'you created a new wallet',
        'view_wallet': 'you viewed your wallet',
}


def get_activity_string( view_name):
    if view_name in activity_dict:
        return activity_dict[view_name]
    else:
        return None
