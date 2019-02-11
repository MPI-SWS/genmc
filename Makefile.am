SUBDIRS = src

.PHONY: test ftest
test:
	$(MAKE) -C src
	cd $(top_builddir)/scripts && ./driver.sh

ftest:
	$(MAKE) -C src
	cd $(top_builddir)/scripts && ./driver.sh --fast
