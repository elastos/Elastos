from django import template

register = template.Library()


@register.filter(name='getval')
def getval(data, key):
    if isinstance(data, dict):
        return data.get(key)
    else:
        return None
