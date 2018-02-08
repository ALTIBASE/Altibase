.PHONY: archive
archive: cmd

.PHONY: install
install: archive

$(BINS): archive
