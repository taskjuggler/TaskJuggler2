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
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
#########################################################################

use XML::Simple;

use PostScript::Simple;

use strict;

use Task;
use TaskList;
package Projet;
use vars qw(@ISA);
@ISA = qw( TaskList );

sub max {
    my ($max) = shift (@_);

    my $temp;
    foreach $temp (@_) {
        $max = $temp if $temp > $max;
    }
    return ($max);
}

sub min {
    my ($min) = shift (@_);

    my $temp;
    foreach $temp (@_) {
        $min = $temp if $temp < $min;
    }
    return ($min);
}

sub new {
    my ( $class, $ref ) = @_;

    
    my $projet = TaskList->new($ref);
    $projet->{xml} = $ref;
 
    bless $projet, $class;

    return $projet;

}


sub extract_list_task {
    my $self = shift;

    # call the TaskList sub
    $self->SUPER::extract_list_task($self->{xml});
}

sub process_tasks {
    my $self = shift;

    # add dependencies lists for all tasks
    $self->add_depends_by_ref($self);

    # place all task in  the grid;
    $self->put_in_grid($self);
    $self->put_in_line;
    $self->set_lin(0);
}

sub get_name {
    my $self = shift;

    return $self->{xml}{Name}
}

sub get_version {
    my $self = shift;

    return $self->{xml}{Version}
}

sub get_end {
    my $self = shift;

    return POSIX::strftime( "%x", localtime( $self->{xml}{end}{content} ) )
}

sub get_start {
    my $self = shift;

    return POSIX::strftime( "%x", localtime( $self->{xml}{start}{content} ) )
}

sub get_now {
    my $self = shift;

    return POSIX::strftime( "%x", localtime( $self->{xml}{now}{content} ) )
}

sub draw {

    my $self = shift;

    my $output_file = shift;

    my $marginx = 1.0; # 1cm
    my $marginy = 1.0; # 1cm
    my $cartouchex = 7.0; # 10cm
    my $cartouchey = 2.0; # 4cm

    # calculate bouding box
    my $bx =
      ( $self->get_max_col + 1 ) * Task->get_task_width() * Task->cell_coef + 2 * $marginx;
    my $by = ( $self->get_height ) * Task->get_task_height() * Task->cell_coef + 2 * $marginy + $cartouchey;

    # create postscript file
    my $p =
      new PostScript::Simple( units => "cm", xsize => $bx, ysize => $by,
        eps => 1 );

    $p->setfont( "Times-Roman-iso", 9 );


    #draw cartouche
#    $p->setlinewidth(0.1);
#    $p->box( 0, 0, $bx, $by);
#    $p->setlinewidth(0.025);
    $p->box( $marginx , $marginy, 
	     $bx - $marginx , $by - $marginy);
	
    my $lineheight = $cartouchey / 4.0;
    my $colwidth = $cartouchex / 4.0;
    $p->box( $marginx , $marginy, 
	     $marginx + $cartouchex , $marginy + $cartouchey);
    # title + version
    $p->line( $marginx, $marginy + $lineheight,
	     $marginx + $cartouchex, $marginy + $lineheight);
    $p->text( $marginx +  $cartouchex / 2.0, $marginy + $lineheight / 3.5,
        $self->get_name()." ".$self->get_version(), 'centre'
    );

    # start, end , now
    $p->line( $marginx, $marginy + 2.0 * $lineheight,
	     $marginx + $cartouchex, $marginy + 2.0 * $lineheight);
    $p->line( $marginx, $marginy + 3.0 * $lineheight,
	     $marginx + $cartouchex, $marginy + 3.0 * $lineheight);
    $p->line( $marginx + $colwidth, $marginy + $lineheight, 
	      $marginx + $colwidth, $marginy + $cartouchey);
    $p->text( $marginx + $colwidth / 2.0, $marginy + $lineheight / 3.5 + $lineheight, "End", 'centre');
    $p->text( $marginx + $colwidth / 2.0, $marginy + $lineheight / 3.5 + 2 * $lineheight, "Start", 'centre');
    $p->text( $marginx + $colwidth / 2.0, $marginy + $lineheight / 3.5 + 3 * $lineheight, "Now", 'centre');
    $p->text( $marginx + 2.5 *$colwidth, $marginy + $lineheight / 3.5 + $lineheight, $self->get_end(), 'centre');
    $p->text( $marginx + 2.5 *$colwidth, $marginy + $lineheight / 3.5 + 2 * $lineheight, $self->get_start(), 'centre');
    $p->text( $marginx + 2.5 *$colwidth, $marginy + $lineheight / 3.5 + 3 * $lineheight, $self->get_now(), 'centre');

# a ajouter : nom du projet + info du projet voir dtd


    # inverse y axis
    $p->{pspages} .= "$marginx u $by $marginy sub u translate \n";

    $self->SUPER::draw( $p, 0 , -by );

    $p->output($output_file);


}


1;
