from django.shortcuts import render

# Create your views here.
def helloWorld(request):
    return render(request, "HelloWorld/HelloWorld.html",{})

