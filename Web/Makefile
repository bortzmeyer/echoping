PAGESHTML=$(shell ls *.wml | grep -v template | sed 's/.wml/.html/' )
DEST=bortz@echoping.sourceforge.net:/home/groups/echoping/htdocs

all: test

%.html:  %.wml template.wml 
	wml -o $@ ${DEBUG}  $< 

test: ${PAGESHTML}
	@for page in ${PAGESHTML}; do \
		(nsgmls -s $$page) \
	done

install: ${PAGESHTML}
	cvs commit
	scp -v ${PAGESHTML} ${DEST}

clean:
	rm -f ${PAGESHTML}
