#
# spec file for package taskjuggler (Stable version)
#
# Copyright (c) 2006 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

Name:           taskjuggler
URL:            http://www.taskjuggler.org
License:        GPL
Group:          Productivity/Office/Other
Summary:        Project management software
Version:        2.4.0
Release:        19
Source0:        taskjuggler-%{version}.tar.bz2 
# Patch1:       fix-gcc41.diff
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

################################################################################
# SuSE, openSUSE
################################################################################
%if 0%{?suse_version}

%if %suse_version > 1020
BuildRequires:  docbook-utils docbook-xsl-stylesheets kdelibs3-devel kdepim3-devel texlive
%else 
BuildRequires:  docbook-utils docbook-xsl-stylesheets kdelibs3-devel kdepim3-devel te_ams
%endif
Requires:       qt3 >= %( echo `rpm -q --queryformat '%{VERSION}' qt3`)
%endif

################################################################################
# Fedora
################################################################################
%if 0%{?fedora_version}
%define debug 0
%define final 0
%define qt_epoch 1
%define kdelibs_epoch 6

%define make_cvs 1

%define disable_gcc_check_and_hidden_visibility 1
BuildRequires:  docbook-utils docbook-xsl-stylesheets kdelibs-devel kdepim-devel tetex
Requires:       qt
%endif

################################################################################
# Mandriva
################################################################################
%if 0%{?mandriva_version}
%define __libtoolize    /bin/true

%define use_enable_final 0
%{?_no_enable_final: %{expand: %%global use_enable_final 0}}

%define compile_apidox 0
%{?_no_apidox: %{expand: %%global compile_apidox 0}}

%define unstable 0
%{?_unstable: %{expand: %%global unstable 1}}

%if %unstable
%define dont_strip 1
%endif
BuildRequires:  docbook-utils openjade kdepim-devel tetex
BuildRequires:  kdelibs-devel >= %{kde_version}
BuildRequires:  libqt3 >= %{qt_version}
Requires: qt3 >= %{qt_version}
%endif


%description
TaskJuggler is a project management tool for Linux and UNIX-like
operating systems. Its new approach to project planning and tracking is
far superior to the commonly used Gantt chart editing tools. It has
already been successfully used in many projects and scales easily to
projects with hundreds of resources and thousands of tasks.

TaskJuggler is an Open Source tools for serious project managers. It
covers the complete spectrum of project management tasks from the first
idea to the completion of the project. It assists you during project
scoping, resource assignment, cost and revenue planing, risk and
communication management.

TaskJuggler provides an optimizing scheduler that computes your project
time lines and resource assignments based on the project outline and
the constrains that you have provided. The build-in resource balancer
and consistency checker offload you from having to worry about
irrelevant details and ring the alarm if the project gets out of hand.
Its flexible "as many details as necessary"-approach allows you to
still plan your project as you go, making it also ideal for new
management strategies such as Extreme Programming and Agile Project
Management.

If you are about to build a skyscraper or just want to put together
your colleague's shift plan for the next month, TaskJuggler is the
right tool for you. If you just want to draw nice looking Gantt charts
to impress your boss or your investors, TaskJuggler might not be right
for you. It takes some effort to master its power, but it will become a
companion you don't want to miss anymore.



Authors:
--------
    Chris Schlaeger <cs@kde.org>,
    Klaas Freitag <freitag@suse.de>
    Lukas Tinkl <lukas.tinkl@suse.cz>

%prep
%setup -q -n taskjuggler-%{version} 
#%patch1
. /etc/opt/kde3/common_options
update_admin --no-unsermake

%build
. /etc/opt/kde3/common_options
./configure \
   $configkde \
 --libdir=/opt/kde3/%_lib \
 --with-qt-libraries=/usr/lib/qt3/%_lib \
 --disable-final
pushd docs
make
popd
make %{?jobs:-j %jobs}

%install
%define tjdocdir  $RPM_BUILD_ROOT/%{_docdir}/taskjuggler/
export DESTDIR=$RPM_BUILD_ROOT
make prefix=/usr install
rm -rf $RPM_BUILD_ROOT/usr/share/doc/HTML
# Install the documentation
install AUTHORS COPYING ChangeLog INSTALL README TODO taskjuggler.lsm %{tjdocdir}
cd docs/en; make install; cd ../../
cp -r Contrib   %{tjdocdir}
# Move taskjuggler away from /opt/kde3
mkdir -p $RPM_BUILD_ROOT/usr/bin/
mv $RPM_BUILD_ROOT/opt/kde3/bin/taskjuggler $RPM_BUILD_ROOT/usr/bin/
mkdir -p $RPM_BUILD_ROOT/usr/%_lib
mv $RPM_BUILD_ROOT/opt/kde3/%_lib/libtaskjuggler* $RPM_BUILD_ROOT/usr/%_lib/
%if 0%{?suse_version}
%if %suse_version > 1000
%suse_update_desktop_file -G "Project Management" taskjuggler ProjectManagement
%else
%suse_update_desktop_file taskjuggler ProjectManagement
%endif
%endif
# install the kate hilighting, cleanup
cd Contrib/kate; make install; cd ../..
rm -rf %{tjdocdir}/Contrib/kate
# remove la files
rm $RPM_BUILD_ROOT/%{_libdir}/libtaskjuggler.la
# Remove not needed files and directories
rm %{tjdocdir}/Contrib/Makefile*
rm %{tjdocdir}/*.html
rm -rf %{tjdocdir}/Contrib/tjGUI
%find_lang %name

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
%{_docdir}/taskjuggler/taskjuggler.ps
%{_docdir}/taskjuggler/Examples
%dir %{_docdir}/taskjuggler/Contrib
%{_docdir}/taskjuggler/Contrib/TJ-Pert
%{_docdir}/taskjuggler/Contrib/tjx2gantt
%{_docdir}/taskjuggler/Contrib/emacs
%{_docdir}/taskjuggler/Contrib/vim
%package kde
Summary:        Project Management Software for KDE
Group:          Productivity/Office/Other
Autoreqprov:    on
Requires:       taskjuggler = %{version}
%description kde
TaskJuggler is a project management tool for Linux and UNIX based
operating systems. Whether you want to plan your college's shifts for
the next month or want to build a skyscraper - TaskJuggler is the tool
for you.

This package provides an XML viewer for files exported by taskjuggler.



Authors:
--------
    Chris Schlaeger <cs@kde.org>,
    Klaas Freitag <freitag@suse.de>
    Lukas Tinkl <lukas.tinkl@suse.cz>


%files kde -f %name.lang
%defattr(-,root,root)
/opt/kde3/bin/TaskJuggler*
/opt/kde3/share/icons/??color/??x??
/opt/kde3/share/icons/crystalsvg/??x??
/opt/kde3/share/apps/katepart/syntax/*
/opt/kde3/share/mimelnk/application/x-tjx.desktop
/opt/kde3/share/mimelnk/application/x-tji.desktop
/opt/kde3/share/mimelnk/application/x-tjp.desktop
/opt/kde3/share/apps/taskjuggler/
/opt/kde3/share/config/taskjugglerrc
/opt/kde3/share/doc/HTML/en/taskjuggler
/usr/share/applications/kde

%changelog -n taskjuggler
* Fri Jul 22 2007 - cs@kde.org
- Update to version 2.4.0
* Mon Jan 01 2007 - cs@kde.org
- Update to version 2.3.1
* Tue Aug 08 2006 - dmueller@suse.de
- fix build
* Wed Jun 21 2006 - dmueller@suse.de
- Remove self-provides taskjuggler-kde (#186079)
* Tue Jun 20 2006 - stbinner@suse.de
- fix build for older distributions
* Wed Jun 14 2006 - dmueller@suse.de
- build parallel
* Wed Feb 15 2006 - stbinner@suse.de
- remove "Software" from GenericName in .desktop file
* Wed Jan 25 2006 - mls@suse.de
- converted neededforbuild to BuildRequires
* Mon Dec 05 2005 - stbinner@suse.de
- Update to version 2.2.0
* Wed Nov 23 2005 - stbinner@suse.de
- Update to version 2.2.0_beta2.
* Sun Nov 06 2005 - cs@suse.de
- Update to version 2.2.0_beta1.
- Cleaned up the spec file. The Perl stuff is no longer needed.
* Thu Oct 13 2005 - stbinner@suse.de
- remove extra qualification for gcc 4.1 compilation
* Tue Oct 11 2005 - stbinner@suse.de
- obviously libkcal/calendarlocal.h is considered internal by
  kdepim developers and so they didn't care api stability
* Mon Aug 29 2005 - adrian@suse.de
- do hide old GUI frontend (#113553)
* Sat Aug 27 2005 - cs@suse.de
- Fixed several critical packaging issues [#113553].
* Mon Aug 15 2005 - cs@suse.de
- Fixed crash with late or missing 'project' section [#104627]
- Enabled test suite again.
- There are no more .cvsignore files, so no need to remove them.
* Sun Aug 14 2005 - cs@suse.de
- Fixed crash in GUI. [#104582]
* Mon Aug 08 2005 - freitag@suse.de
- removed wrong tarball and added the correct one, removed patch
  again
* Mon Aug 08 2005 - freitag@suse.de
- update to rev. 2.1.1
* Mon Aug 01 2005 - freitag@suse.de
- update to rev. 2.1.1 release candidate
* Wed May 25 2005 - freitag@suse.de
- fix the failing test for the Scheduler. Note: I have no solution to
  this but simply removed the testcase completely! TODO!
* Sun May 22 2005 - aj@suse.de
- Fix another gcc4 build problem.
* Tue Apr 19 2005 - ro@suse.de
- build with gcc-4
* Tue Mar 22 2005 - freitag@suse.de
- added fix diff to fix a problem with pathnames [#74224]
* Mon Mar 07 2005 - ro@suse.de
- fixed changelog
* Sun Mar 06 2005 - aj@suse.de
- Fix spec file to include directories.
* Sat Mar 05 2005 - ltinkl@suse.cz
- major update to version 2.1
* Thu Feb 17 2005 - adrian@suse.de
- menu entry moved to xdg dir
* Tue Jan 25 2005 - freitag@suse.de
- update to CVS version as of 2005-01-24
  Brings working version of Chris' graphical taskjuggler tool to the
  preview.
* Mon Nov 22 2004 - ltinkl@suse.cz
- update to CVS version as of 2004-11-22
* Thu Sep 30 2004 - joe@suse.de
- fixed specfile to not create a second icon entry in the KDE menu;
  commenting out the %%suse_update_desktop_file entry was not enough
* Wed Sep 29 2004 - joe@suse.de
- updated to latest version from cvs
- now features latest version of ktjview2
- improved KDE integration (icons, mimetypes)
* Mon Sep 13 2004 - adrian@suse.de
- fix menu entry
* Fri Jul 23 2004 - joe@suse.de
- fixed 64bit and KDE directory issues
* Wed Jul 21 2004 - freitag@suse.de
- update to cvs version that contains the new kde viewer
* Sun Apr 25 2004 - adrian@suse.de
- add qt and kde lib dirs to configure
  (auto-lib suffix detection does not work in this special setup)
* Wed Apr 21 2004 - coolo@suse.de
- build without unsermake
* Thu Mar 18 2004 - freitag@suse.de
- added proper mimetype registration for ktjview
* Tue Mar 09 2004 - freitag@suse.de
- update to version 2.0.1 which contains bugfixes, more flexible
  xml reports, vacations in Html reports.
  Fixes for building on FreeBSD
* Thu Mar 04 2004 - freitag@suse.de
- added a patch for tjx2gantt to work again with MethodMaker perl
  module. MethodMaker had a interface change in the new version
  used from 9.1 on. Added a condition on the patch.
* Thu Nov 27 2003 - freitag@suse.de
- update to version 2.0
* Fri Nov 14 2003 - freitag@suse.de
- update to new snapshot 2003-11-13
* Thu Nov 13 2003 - adrian@suse.de
- add Requires to used qt version
* Mon Nov 10 2003 - freitag@suse.de
- update to CVS version Nov. 2003
* Mon Sep 08 2003 - freitag@suse.de
- update to 1.9.2
* Thu Aug 21 2003 - freitag@suse.de
- updated to another pre2 version
* Wed Aug 06 2003 - freitag@suse.de
- reactivated TestSuite again, works now
* Wed Jul 30 2003 - freitag@suse.de
- on the way to 2.0 another step
* Tue Jul 01 2003 - freitag@suse.de
- added some fixes to build on +kde
* Fri Jun 27 2003 - freitag@suse.de
- update to version 1.9.0
* Tue Jun 17 2003 - coolo@suse.de
- fix build
* Wed May 21 2003 - ro@suse.de
- removed .cvsignore files
- remove unpackaged files
* Tue May 06 2003 - freitag@suse.de
- added a patch older_gcc to fix problems on older versions of
  SuSE Linux with older compilers. This patch can be removed again
  in new versions of taskjuggler
* Tue Mar 11 2003 - freitag@suse.de
- update to 1.4.2 that repairs still some encoding issues, html and
  xml report problems. It brings improved syntax error messages in
  macro use, shows milestones and has a new task attribute.
* Tue Feb 25 2003 - freitag@suse.de
- miniatur changes to the manual and added a index.html to the doc
  directory in order to give the link back to susehelp
* Mon Feb 24 2003 - freitag@suse.de
- update to version 1.4.1 - bugfixes in Localisation issues
  (Bugzilla #23309), speed improvements etc.
* Wed Dec 18 2002 - freitag@suse.de
- fixed a bug in installation that broke building on platforms.
* Wed Dec 18 2002 - freitag@suse.de
- update to version 1.4
* Wed Dec 11 2002 - freitag@suse.de
- removed documentation creation and added the docs as tarfile.
  splitted into different packages for taskjuggler,
  taskjuggler-pstools and taskjuggler-kde
  updated to pre 1.4 version
* Mon Nov 11 2002 - ro@suse.de
- changed neededforbuild <sp> to <opensp>
* Fri Jun 28 2002 - uli@suse.de
- fixed for lib64 archs
* Tue Jun 18 2002 - freitag@suse.de
- replaced acinclude due to problems on some platforms.
* Tue Jun 18 2002 - freitag@suse.de
- update to version 1.2:
  * added a simple KDE viewer for xml output
  * improved html output
  * improved xml output
  * bugfixes
* Fri Jun 14 2002 - freitag@suse.de
- update to version 1.1
* Wed May 22 2002 - coolo@suse.de
- fix path to qt3
* Sun Apr 14 2002 - coolo@suse.de
- fix for gcc 3.1
* Fri Mar 15 2002 - freitag@suse.de
- fixed a nasty bug with vacation, update to version 1.01 for that
* Tue Feb 26 2002 - freitag@suse.de
- update to 1.0.0
  Fixed bug with included files when tjp file is not in working dir
* Fri Feb 22 2002 - freitag@suse.de
- update to 0.9.4 to have proper documentation, correct xml export
  and more bug fixes.
* Thu Feb 07 2002 - freitag@suse.de
- update to version 0.9.1 which brings performance improvements and
  fixes some minor bugs.
* Thu Jan 17 2002 - freitag@suse.de
- first version of taskjugger.
