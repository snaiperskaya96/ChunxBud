#####################################################################
#### Please don't change this file. Use component.mk instead ####
#####################################################################

ifndef SMING_HOME
$(error SMING_HOME is not set: please configure it as an environment variable)
endif

include $(SMING_HOME)/project.mk

include thirdparty/nanopb/extra/nanopb.mk

App-build: before-build

before-build: build-pb-files

build-pb-files:
	$(PROTOC) $(PROTOC_OPTS) --proto_path=thirdparty/proto/ --nanopb_out=app/ thirdparty/proto/proto/*.proto
