#####################################################################
#### Please don't change this file. Use component.mk instead ####
#####################################################################

ifndef SMING_HOME
$(error SMING_HOME is not set: please configure it as an environment variable)
endif

include $(SMING_HOME)/project.mk

include thirdparty/nanopb/extra/nanopb.mk

PROTOBUF_PATH = $(shell pwd)/thirdparty/protobuf/

App-build: before-build

before-build: build-pb-files

build-pb-files:
	./thirdparty/nanopb/generator/nanopb_generator.py -I ./thirdparty/proto ./thirdparty/proto/proto/*.proto -D ./app/
