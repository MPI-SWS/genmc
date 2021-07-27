SUBDIRS = src include

.PHONY: test ftest
test:
	$(MAKE) -C src
	cd $(top_builddir)/scripts && ./driver.sh --debug

ftest:
	$(MAKE) -C src
	cd $(top_builddir)/scripts && ./driver.sh --fast --debug

ctest:
	$(MAKE) -C src
	cd $(top_builddir)/scripts && ./driver.sh --fast

rtest:
	$(MAKE) -C src
	cd $(top_builddir)/scripts && ./randomize-driver.sh
