SUBDIRS=@subdirs@
CONFIGURE_DEPENDENCIES=list-of-dirs.m4 ../configure-common.ac configure-plugins.ac

list-of-dirs.m4: 
	@echo "dnl Created automatically on `date`" > $@
	for file in `ls | grep -v CVS | grep -v autom4te`; do \
		if test -d $$file; then \
			echo "if test \"\$$$${file}_BUILD\" = 1; then" >> $@ ; \
			echo "  AC_CONFIG_SUBDIRS($$file)" >> $@ ; \
			echo "fi" >> $@ ; \
		fi \
	done
