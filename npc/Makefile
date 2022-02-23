BUILD_DIR = ./build
MODULE = GCD



test:
	mill -i __.test

verilog:
	$(call git_commit, "generate verilog")
	mkdir -p $(BUILD_DIR)
	mill -i __.test.runMain Elaborate -td $(BUILD_DIR)

help:
	mill -i __.test.runMain Elaborate --help

compile:
	mill -i __.compile

bsp:
	mill -i mill.bsp.BSP/install

reformat:
	mill -i __.reformat

checkformat:
	mill -i __.checkFormat

clean:
	-rm -rf $(BUILD_DIR)

.PHONY: test verilog help compile bsp reformat checkformat clean

sim:$(BUILD_DIR)/$(GCD).v $(BUILD_DIR)/$(GCD).cpp
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "VERILATOR"
	#@echo "Write this Makefile by yourself."
	verilator -Wall --trace -cc --exe $<

wave:$(BUILD_DIR)/obj_dir/waveform.vcd
	$echo "VIEW WAVES"
	gtkwave $<

$(BUILD_DIR)/obj_dir/waveform.vcd:$(BUILD_DIR)/$(GCD).v $(BUILD_DIR)/$(GCD).cpp
	@$(BUILD_DIR)/obj_dir/V$(MODULE)


include ../Makefile
