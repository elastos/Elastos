package httprestful

import (
	"errors"
	"net/http"
	"regexp"
	"strings"
)

const RouteParams = "route_params"

type params map[string]string

type route struct {
	Method           string
	Path             *regexp.Regexp
	RegisteredParams []string
	Handler          http.HandlerFunc
}

type router struct {
	Routes []*route
}

var paramsRegexp = regexp.MustCompile(`:(\w+)`)

func (r *router) serve(path string, method string) (http.HandlerFunc, params, error) {
	for _, route := range r.Routes {
		if route.Method == method {
			match := route.Path.MatchString(path)
			if match == false {
				continue
			}
			params := params{}
			// check if this route has registered params, and then parse them
			if len(route.RegisteredParams) > 0 {
				params = parseParams(route, path)
			}
			return route.Handler, params, nil

		}
	}
	return nil, params{}, errors.New("route not found")
}

func (r *router) add(method string, path string, handler http.HandlerFunc) {
	route := &route{}
	route.Method = method
	path = "^" + path + "$"
	route.Handler = handler

	if strings.Contains(path, ":") {
		matches := paramsRegexp.FindAllStringSubmatch(path, -1)
		if matches != nil {
			for _, v := range matches {
				route.RegisteredParams = append(route.RegisteredParams, v[1])
				// remove the :params from the url path and replace them with regex
				path = strings.Replace(path, v[0], `(\w+)`, 1)
			}
		}
	}
	compiledPath, err := regexp.Compile(path)
	if err != nil {
		panic(err)
	}
	route.Path = compiledPath
	r.Routes = append(r.Routes, route)
}

func (r *router) Connect(path string, handler http.HandlerFunc) {
	r.add("CONNECT", path, handler)
}

func (r *router) Get(path string, handler http.HandlerFunc) {
	r.add("GET", path, handler)
}

func (r *router) Post(path string, handler http.HandlerFunc) {
	r.add("POST", path, handler)
}

func (r *router) Put(path string, handler http.HandlerFunc) {
	r.add("PUT", path, handler)
}

func (r *router) Delete(path string, handler http.HandlerFunc) {
	r.add("DELETE", path, handler)
}

func (r *router) Head(path string, handler http.HandlerFunc) {
	r.add("HEAD", path, handler)
}

func (r *router) Options(path string, handler http.HandlerFunc) {
	r.add("OPTIONS", path, handler)
}

func parseParams(route *route, path string) params {
	matches := route.Path.FindAllStringSubmatch(path, -1)
	matchedParams := matches[0][1:]

	params := params{}
	for k, v := range matchedParams {
		params[route.RegisteredParams[k]] = v
	}
	return params
}

func getParam(r *http.Request, key string) string {
	ctx := r.Context()
	params := ctx.Value(RouteParams).(params)
	val, _ := params[key]
	return val
}
