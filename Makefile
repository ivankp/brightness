MAIN = brightness

.PHONY: all clean install

all: $(MAIN)

$(MAIN): %: %.c
	gcc -Wall -O3 $< -o $@

clean:
	@rm -fv $(MAIN)

install: $(MAIN)
	@cp -v $(MAIN) $(HOME)/.local/bin

uninstall:
	@rm -fv $(HOME)/.local/bin/$(MAIN)

