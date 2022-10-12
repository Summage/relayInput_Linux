CSRC := $(wildcard *.c)
WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
PROC := $(BUILD_DIR)/relayInput

$(PROC):
	echo $(CSRC)
	

build:
	@mkdir -p $(dir $@)
	@gcc -o $(PROC) $(CSRC) -lpthread

run-env: $(PROC)

run: run-env
	@sudo $(PROC)

clean:
	rm -rf $(BUILD_DIR)
	
.PHONY: build run clean
