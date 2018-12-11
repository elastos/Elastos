package restful

import (
	"io/ioutil"
	"net/http"
	"regexp"
	"strings"

	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

const (
	getIndex = iota
	putIndex
	patchIndex
	deleteIndex
	indexLength
)

type Route struct {
	// The route path in regex
	path   *regexp.Regexp
	params [][]string

	GetHandler    func(params util.Params) (interface{}, error)
	PutHandler    func(params util.Params) (interface{}, error)
	PatchHandler  func(params util.Params) (interface{}, error)
	DeleteHandler func(params util.Params) (interface{}, error)
	PostHandler   func(data []byte) (interface{}, error)
}

// matches find out if the request matches this resource.
func (r *Route) matches(req *http.Request) bool {
	return r.path.MatchString(req.URL.Path)
}

func (r *Route) handle(req *http.Request) (interface{}, error) {
	switch req.Method {
	case http.MethodGet:
		if r.GetHandler != nil {
			return r.GetHandler(r.parseParams(req.URL.Path, getIndex))
		}

	case http.MethodPut:
		if r.PutHandler != nil {
			return r.PutHandler(r.parseParams(req.URL.Path, putIndex))
		}

	case http.MethodPatch:
		if r.PatchHandler != nil {
			return r.PatchHandler(r.parseParams(req.URL.Path, patchIndex))
		}

	case http.MethodDelete:
		if r.DeleteHandler != nil {
			return r.DeleteHandler(r.parseParams(req.URL.Path, deleteIndex))
		}

	case http.MethodPost:
		if r.PostHandler != nil {
			data, err := ioutil.ReadAll(req.Body)
			if err != nil {
				return nil, err
			}
			return r.PostHandler(data)
		}
	}
	return nil, nil
}

func (r *Route) parseParams(path string, index int) util.Params {
	params := r.params[index]
	matches := r.path.FindStringSubmatch(path)
	ret := util.Params{}
	for k, v := range matches[1:] {
		ret[params[k]] = v
	}
	return ret
}

func (r *Route) SetParams(method string, params ...string) *Route {
	switch method {
	case http.MethodGet:
		r.params[getIndex] = params

	case http.MethodPut:
		r.params[putIndex] = params

	case http.MethodPatch:
		r.params[patchIndex] = params

	case http.MethodDelete:
		r.params[deleteIndex] = params

	}
	return r
}

func (r *Route) SetHandler(method string, handler interface{}) *Route {
	switch method {
	case http.MethodGet:
		r.SetGetHandler(handler.(func(util.Params) (interface{}, error)))

	case http.MethodPut:
		r.SetPutHandler(handler.(func(util.Params) (interface{}, error)))

	case http.MethodPatch:
		r.SetPatchHandler(handler.(func(util.Params) (interface{}, error)))

	case http.MethodDelete:
		r.SetDeleteHandler(handler.(func(util.Params) (interface{}, error)))

	case http.MethodPost:
		r.SetPostHandler(handler.(func([]byte) (interface{}, error)))

	}
	return r
}

func (r *Route) SetGetHandler(handler func(util.Params) (interface{}, error)) *Route {
	r.GetHandler = handler
	return r
}

func (r *Route) SetPutHandler(handler func(util.Params) (interface{}, error)) *Route {
	r.PutHandler = handler
	return r
}

func (r *Route) SetPatchHandler(handler func(util.Params) (interface{}, error)) *Route {
	r.PatchHandler = handler
	return r
}

func (r *Route) SetDeleteHandler(handler func(util.Params) (interface{}, error)) *Route {
	r.DeleteHandler = handler
	return r
}

func (r *Route) SetPostHandler(handler func(data []byte) (interface{}, error)) *Route {
	r.PostHandler = handler
	return r
}

func NewRoute(regex *regexp.Regexp) *Route {
	return &Route{
		path:   regex,
		params: make([][]string, indexLength),
	}
}

func ParseUrl(url string) (*regexp.Regexp, []string) {
	var params []string
	if strings.Contains(url, ":") {
		matches := regexp.MustCompile(`:(\w+)`).FindAllStringSubmatch(url, -1)
		for _, v := range matches {
			params = append(params, v[1])
			url = strings.Replace(url, v[0], `(\w+)`, 1)
		}
	}
	return regexp.MustCompile("^" + url + "$"), params
}

func matchHandler(method string, handler interface{}) bool {
	switch method {
	case http.MethodGet, http.MethodPut, http.MethodPatch, http.MethodDelete:
		_, ok := handler.(func(params util.Params) (interface{}, error))
		return ok

	case http.MethodPost:
		_, ok := handler.(func(data []byte) (interface{}, error))
		return ok
	}

	return false
}
