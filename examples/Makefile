ROOT := $(abspath ../..)

EXAMPLES := output input adc can_tx can_rx can_tod pwm spi iob eth_udp pid fm error uart printf

.PHONY: $(EXAMPLES) clean

$(EXAMPLES):
	$(MAKE) -C $(ROOT) -f fse_pb_bsp/examples/examples.mk EXAMPLE=$@

clean:
	rm -rf build
