.PHONY: all
all: clean build

.PHONY: build
build: deps
	go build -o build/base58 ./cmd/base58

.PHONY: install
install: deps
	go install ./...

.PHONY: deps
deps:
	go get -d -v ./...

.PHONY: test
test: testdeps build
	go test -v ./...

.PHONY: testdeps
testdeps:
	go get -d -v -t ./...
	go get -u golang.org/x/lint/golint

.PHONY: lint
lint: testdeps
	go vet ./...
	golint -set_exit_status ./...

GOFMT_RET = .gofmt.txt
.PHONY: gofmt
gofmt: testdeps
	rm -f $(GOFMT_RET)
	gofmt -s -d *.go | tee $(GOFMT_RET)
	test ! -s $(GOFMT_RET)

.PHONY: clean
clean:
	rm -rf build
	go clean
