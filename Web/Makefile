PAGESHTML=$(shell ls *.wml | grep -v template | sed 's/.wml/.html/' )
DEST=bortz@echoping.sourceforge.net:/home/groups/echoping/htdocs
URL=http://echoping.sourceforge.net/

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

checkbot:
	checkbot --url ${URL} --match ${URL} \
		--verbose 

clean:
	rm -f ${PAGESHTML}
