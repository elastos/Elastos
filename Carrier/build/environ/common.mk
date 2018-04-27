# command

BUILD_DIR = $(abspath $(dir $(abspath $(lastword $(MAKEFILE_LIST))))/..)
ROOT_DIR  = $(abspath $(BUILD_DIR)/../)

DIST_DIR  = $(BUILD_DIR)/_dist/$(HOST)-$(ARCH)/$(BUILD)
DEPS_DIR  = $(BUILD_DIR)/_build/$(HOST)-$(ARCH)/$(BUILD)
REPO_DIR  = $(BUILD_DIR)/.tarballs
