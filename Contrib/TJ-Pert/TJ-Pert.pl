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

use strict;

my $file       = shift;
my $input_file = $file;
$file =~ s/xml$//;
my $output_file = $file . "eps";

my $projetxml =
  XMLin( $input_file, forcearray => [ "Task", "TaskID", "Previous" ] );

#print Dumper($projetxml);

my $bx;
my $by;

# create global TaskList
my $tasks = TaskList->new($projetxml);

# Extract task from xml/perl struct
$tasks->extract_list_task($projetxml);

# add dependenvcies lists for all tasks
$tasks->add_depends_by_ref($tasks);

# place all task in  the grid;
$tasks->put_in_grid($tasks);
$tasks->put_in_line;

print "Creating $output_file\n";

draw_net($tasks);

sub draw_net {
    my $tasklist = shift;

    # calculate bouding box
    $bx =
      ( $tasklist->get_max_col + 1 ) * Task->get_task_width() * Task->cell_coef;
    $by = ( $tasklist->get_height ) * Task->get_task_height() * Task->cell_coef;

    # create postscript file
    my $p =
      new PostScript::Simple( units => "cm", xsize => $bx, ysize => $by,
        eps => 1 );

    $p->setfont( "Times-Roman", 9 );

    $p->{pspages} .= " 0 $by u translate \n";
    $tasklist->draw( $p, 0, -by );

    $p->output($output_file)

}
