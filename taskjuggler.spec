#
# spec file for package taskjuggler (CVS version)
# (this file assumes that you have generated the source tarballs with the 
# "createtarball" script from the 'admin' directory)
#
# FIXME: everything goes into /opt/kde3 prefix for now
#
Name:          taskjuggler
URL:           http://www.taskjuggler.org
License:       GPL
Group:         Productivity/Office/Other
Summary:       Project management software
Version:       cvs
Release:       1
Source0:       taskjuggler-%{version}.tar.bz2
Source1:       manual-%{version}.tar.bz2
Requires:      qt3 >= %( echo `rpm -q --queryformat '%{VERSION}' qt3`)
Requires:      kdelibs3
BuildRequires: libxml2, libxml2-devel, libxslt, libxslt-devel, tetex
Autoreqprov:   on
BuildRoot:     %{_tmppath}/%{name}-%{version}-build

%description
TaskJuggler is a project management tool for Linux and UNIX based
operating systems. Whether you want to plan your college's shifts for
the next month or want to build a skyscraper - TaskJuggler is the tool
for you.

Instead of clicking painfully through hundreds of dialog boxes you
specify your TaskJuggler project in a simple text format. You simply
list all your tasks and their dependencies. The information is sent
through TaskJuggler and you will get all sorts of reports in HTML or
XML format.

TaskJuggler not only honors the task interdependencies but also takes
resource constraints into account. Using TaskJuggler's powerful
filtering and reporting algorithms you can create task lists, resource
usage tables, status reports, project calendars, and project accounting
statements.



Authors:
--------
    Chris Schaeger <cs@suse.de>,
    Klaas Freitag <freitag@suse.de>

%prep
%setup -q -n taskjuggler-%{version}

%build
./configure \
 --prefix=/opt/kde3 \
 --libdir=/opt/kde3/%_lib \
 --with-qt-libraries=/usr/lib/qt3/%_lib \
 --disable-final \
 --with-kde-support=yes
make

%install
rm -rf $RPM_BUILD_ROOT
%define tjdocdir $RPM_BUILD_ROOT/%{_docdir}/taskjuggler/
export DESTDIR="$RPM_BUILD_ROOT"
make install

# Install the documentation
install AUTHORS COPYING ChangeLog INSTALL README TODO taskjuggler.lsm %{tjdocdir}

# The documentation
rm -rf %{tjdocdir}/manual
tar --no-same-owner -xjf %{SOURCE1} -C %{tjdocdir}
mv %{tjdocdir}/manual-%{version} %{tjdocdir}/manual
cp -r Contrib   %{tjdocdir}
cp -r TestSuite %{tjdocdir}
cp %{SOURCE2} %{tjdocdir}
bunzip2  %{tjdocdir}/*bz2

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/taskjuggler
%{_libdir}/libtaskjuggler*
%dir %{_docdir}/taskjuggler
%{_docdir}/taskjuggler/AUTHORS
%{_docdir}/taskjuggler/COPYING
%{_docdir}/taskjuggler/ChangeLog
%{_docdir}/taskjuggler/INSTALL
%{_docdir}/taskjuggler/README
%{_docdir}/taskjuggler/TODO
%{_docdir}/taskjuggler/taskjuggler.lsm
# directories:
%{_docdir}/taskjuggler/manual
%{_docdir}/taskjuggler/Examples
%{_docdir}/taskjuggler/Contrib
%{_docdir}/taskjuggler/TestSuite

%package kde
Provides:     taskjuggler-kde
Summary:      Project Management Software for KDE
Group:        Productivity/Office/Other
Autoreqprov:  on
BuildRequires: qt3-devel, kdelibs3-devel

%description kde
TaskJuggler is a project management tool for Linux and UNIX based
operating systems. Whether you want to plan your college's shifts for
the next month or want to build a skyscraper - TaskJuggler is the tool
for you.

This package provides tools for viewing output produced by taskjuggler.



Authors:
--------
    Chris Schlaeger <cs@suse.de>,
    Klaas Freitag <freitag@suse.de>
    Lukas Tinkl <lukas.tinkl@suse.cz>


%files kde
%defattr(-,root,root)
/opt/kde3/bin/ktjview*
/opt/kde3/%_lib/kde3/libktjviewpart*
/opt/kde3/share/icons/*
/opt/kde3/share/apps/ktjview*
/opt/kde3/share/services/ktjview.desktop
/opt/kde3/share/mimelnk/application/x-tjxml.desktop
