GO := go
BINARY_DIR := dist
CRYPHPTOR_BINARY := $(BINARY_DIR)/cryphptor
PHPEXT_BUILDER_BINARY:= $(BINARY_DIR)/phpext_builder

LDFLAGS := -ldflags "-s -w"

.PHONY: all build test clean install cryphptor phpext_builder docker run fmt vet lint

all: build

build: cryphptor phpext_builder
	@echo "All binaries built successfully"

cryphptor:
	@mkdir -p $(BINARY_DIR)
	$(GO) build $(LDFLAGS) -o $(CRYPHPTOR_BINARY) ./cmd/cryphptor

phpext_builder:
	@mkdir -p $(BINARY_DIR)
	$(GO) build $(LDFLAGS) -o $(PHPEXT_BUILDER_BINARY) ./cmd/phpext_builder

test: 
	$(GO) test -v ./...

clean:
	rm -rf $(BINARY_DIR)

install:
	$(GO) mod tidy
	$(GO) mod download

fmt:
	$(GO) fmt ./...

vet:
	$(GO) vet ./...

lint:
	golangci-lint run

docker: cryphptor-docker docker-latest docker-74 docker-84

cryphptor-docker:
	docker build -t cryphptor-cryphptor -f Dockerfile.cryphptor .

docker-latest:
	docker build -t cryphptor-phpfpm -f Dockerfile.phpfpm .

docker-74:
	docker build -t cryphptor-phpfpm-7.4 -f Dockerfile.phpfpm-7.4 .

docker-84:
	docker build -t cryphptor-phpfpm-8.4 -f Dockerfile.phpfpm-8.4 .

run: loader
	$(LOADER_BINARY) --config=config.yaml

