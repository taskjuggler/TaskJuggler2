TJ-Pert - a Pert diagram generator for TaskJuggler
--------------------------------------------------

Introduction:
-------------
TJ-Pert is a backend for TaskJuggler. This program is used to generate an eps file from the TaskJuggler XML output.
The result can be included in a document by another program (OpenOffice, TeX..)

Each task is shown as a rectangle including several pieces of information. Actually it contains:
- the start date
- the end date
- the task name
- a gray scale bar showing tasks completness

More informations will be added in future releases earliest start and finish dates, latest start and finish dates

TaskJuggler can define container task. Tasks belonging to another task or grouped in a gray box.

Project is shown in a cartouche with à legend including:
- creation date
- project's start date
- project's end date
- project's name

Installation:
-------------
TJ-Pert is writen in Perl 5.6, it uses the following modules:

- FindBin
- XML::Simple
- PostScript::Simple

You can find this modules on CPAN:
http://www.cpan.org

Once you have installed these modules you can install TJ-Pert wherever you want:

cd <install-dir>
tar xzvf TJ-Pert.tgz


Running:
--------

Add the install dir in your path:

PATH=<install-dir>/TJ-Pert:$PATH

Generate the xml output of TaskJuggler for your project. To do this add the following tag in your project file:

xmltaskreport "filename.xml"

then run TJ-Pert:

TJ-Pert.pl filename.xml


The file filename.eps will be created. You can preview it by using gv.


TODO:
-----

- More informations for each task
- more parameters on command line
etc....


Author:
-------

Philippe Midol-Monnet
philippe@midol-monnet.org