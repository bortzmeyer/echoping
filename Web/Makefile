PAGESHTML=$(shell ls *.wml | grep -v template | sed 's/.wml/.html/' )
OTHERPAGES=$(shell ls *.sh)
DEST=bortz@echoping.sourceforge.net:/home/groups/e/ec/echoping/htdocs
URL=http://echoping.sourceforge.net/

all: test

%.html:  %.wml template.wml 
	wml -o $@ ${DEBUG}  $< 

test: ${PAGESHTML}
	@for page in ${PAGESHTML}; do \
		(nsgmls -s $$page) \
	done

install: ${PAGESHTML}
	svn commit
	scp -v ${PAGESHTML} ${OTHERPAGES} ${DEST}

checkbot:
	checkbot --url ${URL} --match ${URL} \
		--verbose 

clean:
	rm -f ${PAGESHTML}

# $Id$
