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

use XML::Simple;

use PostScript::Simple;

use strict;

use Task;

package TaskList;
use vars qw(@ISA);
@ISA = qw( Task );

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

    my $tasklist = Task->new($ref);
    $tasklist->{Liste}     = [];
    $tasklist->{Max_X}     = -1;
    $tasklist->{Min_X}     = 999;
    $tasklist->{Abs_Max_X} = -1;
    $tasklist->{Abs_Max_Y} = -1;

    bless $tasklist, $class;

    return $tasklist;

}

sub add_task {
    my $self = shift;

    return push @{ $self->{List} }, shift;
}

sub get_abs_max_x {
    my $self = shift;

    return $self->{AbsMaxX};
}

sub get_abs_max_y {
    my $self = shift;

    return $self->{AbsMaxY};
}

# extract each task from the xml structure in a TaskList
sub extract_list_task {
    my $self = shift;

    # xml/perl struct
    my $xmllist = shift;

    my $task;
    my $taskl;

    foreach $task ( @{ $xmllist->{Task} } ) {
        if ( $task->{SubTasks} ) {

            # the task have subtasks => create new TaskList
            $taskl = TaskList->new($task);

            # extract subtasks from xml/perl struct
            $taskl->extract_list_task( $task->{SubTasks} );

            # add the new TaskList
            $self->add_task($taskl);

        }
        else {
            $self->add_task( Task->new($task) );

        }

        #      print "Id: ",$task->{"Id"}, "\n";
    }

}

# Find a task whith its Id
sub find_id {
    my $self    = shift;
    my $task_id = shift;

    my $task;

    foreach $task ( @{ $self->{List} } ) {
        return $task if ( $task_id eq $task->get_id() );
        if ( ref($task) eq "TaskList" ) {
            my $result = $task->find_id($task_id);
            return $result if ($result);
        }
    }

    return;

}

sub add_depends_by_ref {
    my $self     = shift;
    my $alltasks = shift;

    my $lst_dep;

    my $task;
    foreach $task ( @{ $self->{List} } ) {
      $lst_dep = $task->find_dep_lst($alltasks);
      $task->set_dep($lst_dep);
      if ( $task->is_container )
	{
	  # traite récursivement les taches de la liste
	  $task->add_depends_by_ref($alltasks);
	  
	  # ajoute les dépendance de la liste à la premiere sous-tache
	  if ( $lst_dep )
	    {
	      my $first = $task->first_subtask();
	      $first->set_dep($lst_dep);
	    }
	}
    }
}

# TaskList is a container
sub is_container {
    my $self = shift;

    return 1;
}

sub get_max_col {
    my $self = shift;

    return $self->{Max_X};
}

sub get_min_col {
    my $self = shift;

    return $self->{Min_X};
}

sub get_height {
    my $self = shift;

    return $self->{Height};
}

sub set_height {
    my $self = shift;

    $self->{Height} = shift;
}

# Find latest sub task
sub last_subtask {
    my $self = shift;

    my $id = $self->get_id();
    my $lastsub;
    my $date = 0;
    my $subtask;

    foreach $subtask ( @{ $self->{List} } ) {
        if ( $subtask->get_end() > $date ) {
            $lastsub = $subtask;
            $date    = $subtask->get_end();
        }
    }

    $lastsub = $lastsub->last_subtask() if ( $lastsub->is_container() );

    return $lastsub;
}

# Find first subtask
sub first_subtask {
    my $self = shift;

    my $id = $self->get_id();
    my $firstsub;
    my $date;
    my $subtask;

    $firstsub = $self->{List}->[0];
    $date = $firstsub->get_start();
    foreach $subtask ( @{ $self->{List} } ) {
        if ( $subtask->get_start() < $date ) {
            $firstsub = $subtask;
            $date    = $subtask->get_start();
        }
    }

    $firstsub = $firstsub->first_subtask() if ( $firstsub->is_container() );

    return $firstsub;
}

# return true if the cell is free in local grid
sub cell_is_free {
    my $self       = shift;
    my $line       = shift;
    my $height     = shift;
    my $column_min = shift;
    my $column_max = shift;

    # intersection coordinates
    my $x_min;
    my $y_min;
    my $x_max;
    my $y_max;

    my $task;

    foreach $task ( @{ $self->{List} } ) {
        if ( $task->get_lin != -1 ) {
            $x_min = max( $column_min, $task->get_min_col );
            $x_max = min( $column_max, $task->get_max_col );
            $y_min = max( $line,       $task->get_lin );
            $y_max =
              min( $line + $height - 1,
                $task->get_lin + $task->get_height - 1 );
            return 0 if ( $x_min <= $x_max and $y_min <= $y_max );
        }
    }

    return 1;
}

sub col_is_set {
    my $self = shift;

    my $task;
    foreach $task ( @{ $self->{List} } ) {
        return 0 if ( !$task->col_is_set );
    }
    return 1;
}

# find first free line in column $column since line $line
sub find_free_line {
    my $self   = shift;
    my $column = shift;
    my $line   = shift;

    while ( !$self->cell_is_free( $column, $line ) ) {
        $line++;
    }

    return $line;
}

# Place each task in the grid
sub put_in_grid {
    my $self = shift;

    my $alltasks = shift;

    my $again = 1;
    my $task;

    my $col;
    my $line;

    my $againrec = 0;

    while ($again) {
        $again = 0;

        foreach $task ( @{ $self->{List} } ) {
            if ( $task->is_container ) {
                $again = $task->put_in_grid($alltasks);
                $self->{Max_X} = $task->get_max_col
                  if ( $self->{Max_X} < $task->get_max_col );

                $self->{Min_X} = $task->get_min_col
                  if ( $self->{Min_X} > $task->get_min_col );

            }
            else {
                if ( !$task->col_is_set() && $task->can_set_col() ) {

                    # find column and line in the grid
                    $col = $task->set_col();

                    # update the last line and column
                    $self->{Max_X} = $col if ( $self->{Max_X} < $col );

                    $self->{Min_X} = $col if ( $self->{Min_X} > $col );

                    $again    = 1;
                    $againrec = 1;

                }
            }
        }
    }

    return $againrec;

}

# draw recursively all the tasks and dependencies
sub draw {
    my $self = shift;

    #postscript output
    my $p = shift;

    my $xpos = shift;
    my $ypos = shift;

    my $task;

    # cell height
    my $c_h = Task::get_task_height() * Task->cell_coef();

    # cell width
    my $c_w = Task::get_task_width() * Task->cell_coef();

    #cell margins
    my $m_x = ( $c_w - Task::get_task_width() ) / 2.0;
    my $m_y = ( $c_h - Task::get_task_height() ) / 2.0;

    # draw dependencies lines
    my ( $x_rs, $y_rs, $x_ls, $y_ls );
    my $dep;

    foreach $task ( @{ $self->{List} } ) {
        $x_rs = $task->get_min_col() * $c_w + $m_x;
        $y_rs = -( ( $task->get_lin() + 0.5 ) * $c_h );
        foreach $dep ( @{ $task->get_dep() } ) {
            $x_ls = ( $dep->get_max_col() + 1 ) * $c_w - $m_x;
            $y_ls = -( $dep->get_lin() * $c_h + 0.5 * $c_h );
            $p->line( $x_ls, $y_ls, $x_rs, $y_rs );
        }
    }

    # draw tasks
    foreach $task ( @{ $self->{List} } ) {

        #      if ($task->get_lin() != -1 && $task->get_col() != -1) {
        if ( $task->get_lin() != -1 ) {

            # task position in the chart
            my $x_pos = $task->get_col() * $c_w + $m_x;
            my $y_pos = -( ( $task->get_lin() + 1 ) * $c_h - $m_y );

            $task->draw( $p, $x_pos, $y_pos );
        }

        $p->setcolour("grey70");

        # draw rectangle for container
        if ( $task->get_lin() != -1 ) {
            $p->box( $self->get_min_col() * $c_w + 0.5 * $m_x,
                -( $self->get_lin * $c_h + 0.5 * $m_y ),
                ( $self->get_max_col + 1 ) * $c_w - 0.5 * $m_x,
                -( ( ( $self->get_lin + $self->get_height ) * $c_h ) - 0.5 *
                $m_y ) );
        }
        $p->setcolour("black");
    }

}

# each task are put in a relative line
sub put_in_line {
    my $self = shift;

    my $task;

    foreach $task ( @{ $self->{List} } ) {
        $task->put_in_line if ( $task->is_container );
        my $line = 0;
        while (
            !$self->cell_is_free( $line, $task->get_height, $task->get_min_col,
                $task->get_max_col ) )
        {
            $line++;
        }
        $task->set_lin($line);
        if ( $self->get_height < $line + $task->get_height ) {
            $self->set_height( $line + $task->get_height );
        }
    }
}

# set line for a TaskList
# all the subtasks are in relative coordinate => they are moved down
sub set_lin {
    my $self = shift;
    my $line = shift;

    my $task;

    $self->SUPER::set_lin($line);

    foreach $task ( @{ $self->{List} } ) {
        $task->add_lin($line);
    }

}

sub add_lin {

    my $self = shift;
    my $line = shift;

    my $task;

    $self->SUPER::add_lin($line);

    foreach $task ( @{ $self->{List} } ) {
        $task->add_lin($line);
    }

}

1;
