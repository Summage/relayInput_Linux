CSRC := $(wildcard *.c)
WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
PROC := $(BUILD_DIR)/relayInput

build: 
	@mkdir -p $(BUILD_DIR)
	@gcc -o $(PROC) $(CSRC) -lpthread

run-env: $(PROC)

run: run-env
	@sudo $(PROC)

clean:
	rm -rf $(BUILD_DIR)
	
.PHONY: build run clean
