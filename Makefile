all: compute.o  #pdf

clean:
	rm -rf *o ^compute.* ^brute.* main.pdf main.ps

compute.o:
	g++ -ggdb3 -Wall compute.cpp -o compute

TARGET=main
HTML=main_html

default: pdf

both: pdf html

dvi: ${TARGET}.tex
#   run latex twice to get references correct
	latex ${TARGET}.tex
#   you can also have a bibtex line here
#   bibtex $(TARGET).tex
	latex $(TARGET).tex

ps: dvi
	dvips -R -Poutline -t letter ${TARGET}.dvi -o ${TARGET}.ps

pdf: ps
	ps2pdf ${TARGET}.ps


html:
	cp ${TARGET}.tex ${HTML}.tex
	latex ${HTML}.tex
	latex2html -split 0 -show_section_numbers -local_icons -no_navigation ${HTML}

	sed 's/<\/SUP><\/A>/<\/SUP><\/A> /g' < ${HTML}/index.html > ${HTML}/index2.html
	mv ${HTML}/index2.html ${HTML}/index.html
	rm ${HTML}.*
