man_MANS = taskjuggler.1 TaskJugglerUI.1
EXTRA_DIST = $(man_MANS)

.xml.1:
	$(DOCBOOK_TO_MAN) --to-stdout $< > $@

clean-local:
	rm -f *.1

