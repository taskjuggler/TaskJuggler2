#!/usr/bin/perl
######################################################################## 
# Copyright (c) 2002 by Philippe Midol-Monnet <philippe@midol-monnet.org>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#########################################################################

use FindBin;

use lib $FindBin::Bin ;

use XML::Simple;

#use Data::Dumper;

use PostScript::Simple;

use Task;
use TaskList;
use Projet;

use strict;

my $file       = shift;
my $input_file = $file;

# New suffix for xml file: tjx
# keep the old one in case...
if ($file =~ /tjx$/) {
  $file =~ s/tjx$//;
}
else {
  $file =~ s/xml$//;
}

my $output_file = $file . "eps";

my $projetxml =
  XMLin( $input_file, forcearray => [ "Task", "TaskID", "Previous" ] );

#print Dumper($projetxml);


# create project
print "Analysing $input_file\n";
my $projet = Projet->new($projetxml);

# Extract task from xml/perl struct
$projet->extract_list_task($projetxml);

$projet->process_tasks;



print "Creating $output_file\n";

$projet->draw($output_file);

