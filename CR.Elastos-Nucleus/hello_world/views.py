from django.shortcuts import render

# Create your views here.
def hello_World(request):
    return render(request, "HelloWorld/HelloWorld.html",{})

