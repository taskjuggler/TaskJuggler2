man_MANS = taskjuggler.1 TaskJugglerUI.1
EXTRA_DIST = $(man_MANS)

.xml.1:
	if test "${DOCBOOK_TO_MAN}" != "no" ; then \
	  $(DOCBOOK_TO_MAN) --to-stdout $< > $@ ;\
	fi  

clean-local:
	rm -f *.1

