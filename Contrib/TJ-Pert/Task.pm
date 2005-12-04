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

use strict;
use POSIX;

package Task;

# Parameters

my $task_width  = 4;      # 4cm
my $task_height = 1.5;    # 1,5 cm
my $cell_coef   = 1.3;

sub get_task_width {
    return $task_width;
}

sub get_task_height {
    return $task_height;
}

sub cell_coef {
    return $cell_coef;
}

sub new {
    my ( $class, $ref ) = @_;

    my $task = {
        Task => $ref,
        Col  => -1,
        Lin  => -1,
    };

    bless $task, $class;

    return $task;

}

sub get_id {
    my $self = shift;

    return $self->{Task}->{Id};

}

sub set_dep {
    my $self = shift;

    my $dep = shift;

    $self->{Dep} = $dep;
}

sub can_set_col {
    my $self = shift;

    my $task_dep;

    foreach $task_dep ( @{ $self->{Dep} } ) {
        return 0 if ( !$task_dep->col_is_set() );
    }

    return 1;

}

sub set_col {
    my $self = shift;

    my $column = -1;
    my $task_dep;
    my $col_task;

    foreach $task_dep ( @{ $self->{Dep} } ) {

        $col_task = $task_dep->get_max_col();
        $column = $col_task if ( $col_task > $column );
    }

    $column++;

    $self->{Col} = $column;

    return $column;
}

sub col_is_set {
    my $self = shift;

    return ( $self->{Col} != -1 );
}

sub get_col {
    my $self = shift;

    return $self->{Col};
}

sub get_min_col {
    my $self = shift;

    return $self->{Col};
}

sub get_max_col {
    my $self = shift;

    return $self->{Col};
}

sub calc_lin {
    my $self = shift;

    my $first_dep;
    my $line = 0;

    if ( $self->{Dep}[0] ) {
        $first_dep = $self->{Dep}[0];
        $line      = $first_dep->get_lin();
    }

    return $line;
}

sub set_lin {
    my $self = shift;
    my $line = shift;

    $self->{Lin} = $line;

    return;
}

sub add_lin {
    my $self = shift;
    my $line = shift;

    $self->{Lin} = $self->{Lin} + $line;

    return;
}

sub get_lin {
    my $self = shift;

    return $self->{Lin};
}

sub get_height {
    return 1;
}

sub set_abs_lin {
    my $self = shift;
    my $line = shift;

    $self->{Abs_Lin} = $line;

    return;
}


# return task dependencies
sub get_dep {
    my $self = shift;

    return $self->{Dep};
}

## return task dependencie id from xml
#sub get_dep_id
#  {
#    my $self = shift;

#    return $self->{Task}->{Depends}{TaskID};
#  }

# return task dependencie id from xml
sub get_previous_id {
    my $self = shift;

    return $self->{Task}->{Previous};
}

# return task dependencie id from xml
sub get_follower_id {
    my $self = shift;

    return $self->{Task}->{Followers};
}

sub is_container {
    my $self = shift;

    return 0;
}

sub get_parent {
    my $self = shift;

    return $self->{Task}->{ParentTask};

}

sub get_end {
    my $self = shift;

    if ($self->{Task}->{actualEnd}{content} != 0)
      {
	return $self->{Task}->{actualEnd}{content}
      }
    else 
      {
	return $self->{Task}->{planEnd}{content};
      }
}

sub get_start {
    my $self = shift;
    if ($self->{Task}->{actualStart}{content} != 0)
      {
	return $self->{Task}->{actualStart}{content}
      }
    else 
      {
        return $self->{Task}->{planStart}{content};
      }

}

sub draw {
    my $self   = shift;
    my $out_ps = shift;
    my $x_pos  = shift;
    my $y_pos  = shift;

    #  print $x_pos, " ",$y_pos, "\n";

    # blank zone
    $out_ps->setcolour("white");
    $out_ps->box( $x_pos, $y_pos, $x_pos + $task_width, $y_pos + $task_height,
        1 );

    # draw rectangle
    $out_ps->setcolour("black");
    $out_ps->box( $x_pos, $y_pos, $x_pos + $task_width, $y_pos + $task_height );

    my $x3          = $task_width / 3.0;
    my $y4          = $task_height / 4.0;
    my $text_margin = $y4 / 4.0;

    if ($self->{Task}->{Type} eq "Milestone" )
	{
	    $out_ps->box($x_pos + $x3, $y_pos + 3 * $y4, $x_pos + 2 * $x3, $y_pos + $task_height);
	}
	else
	{
    # draw each rectangle inside 
    $out_ps->line( $x_pos, $y_pos + $y4, $x_pos + $task_width, $y_pos + $y4 );
    $out_ps->line( $x_pos, $y_pos + 3 * $y4, $x_pos + $task_width,
        $y_pos + 3 * $y4 );
    $out_ps->line( $x_pos + $x3, $y_pos, $x_pos + $x3, $y_pos + $y4 );
    $out_ps->line( $x_pos + 2 * $x3, $y_pos, $x_pos + 2 * $x3, $y_pos + $y4 );
    $out_ps->line( $x_pos + $x3, $y_pos + 3 * $y4, $x_pos + $x3,
        $y_pos + $task_height );
    $out_ps->line(
        $x_pos + 2 * $x3, $y_pos + 3 * $y4,
        $x_pos + 2 * $x3, $y_pos + $task_height
    );
	}

    # draw progress bar (% complete)
    $out_ps->setcolour("grey80");
    $out_ps->box( $x_pos, 
		  $y_pos + $y4, 
		  $x_pos + $task_width * $self->{Task}->{complete} / 100.0,
		  $y_pos + 2 * $y4
		  ,1
		);
    $out_ps->setcolour("black");

    # write text
    # Task ID
    $out_ps->text(
        $x_pos + $task_width / 2.0, $y_pos + 2 * $y4 + $text_margin,
        $self->get_id(),            'centre'
    );

	
    # Start
    my $start_value;
    if ($self->{Task}->{actualStart}{content} != 0)
      {
	$start_value = $self->{Task}->{actualStart}{content}
      }
    else 
      {
	$start_value = $self->{Task}->{planStart}{content};
      }
    my $start = POSIX::strftime( "%x", localtime( $start_value ) );

   if ($self->{Task}->{Type} eq "Milestone" )
	{
    $out_ps->text(
        $x_pos + $x3 + $x3 / 2.0, $y_pos + 3 * $y4 + $text_margin,
        $start,             'centre'
    );
    }
    else
    {
    $out_ps->text(
        $x_pos + $x3 / 2.0, $y_pos + 3 * $y4 + $text_margin,
        $start,             'centre'
    );
    }
    # End

    my $end_value;
    if ($self->{Task}->{actualEnd}{content} != 0)
      {
	$end_value = $self->{Task}->{actualEnd}{content}
      }
    else 
      {
	$end_value = $self->{Task}->{planEnd}{content};
      }
    my $end = POSIX::strftime( "%x", localtime( $end_value ) );

   if ($self->{Task}->{Type} ne "Milestone" )
	{
    $out_ps->text( $x_pos + 2 * $x3 + $x3 / 2.0,
        $y_pos + 3 * $y4 + $text_margin, $end, 'centre' );
        }
}

# compute absolute Id from relative Id
sub id_to_abs {
    my $self   = shift;
    my $rel_id = shift;

    my $abs_id;
    my $task_id = $self->get_id;

    my @lst_sub_id = split /\./, $task_id;

    while ( $rel_id =~ s/^\!(.*)/$1/ ) {
        pop @lst_sub_id;
    }

    $abs_id = join '.', ( @lst_sub_id, $rel_id );
    return $abs_id;
}

#create dependencies (previous tasks) reference list
sub find_dep_lst {
    my $self     = shift;
    my $alltasks = shift;

    my $dep_ref;
    my $dep_id;
    my @lst_dep;
    my $abs_dep;

    if ( $self->get_previous_id ) {
        foreach $dep_id ( @{ $self->get_previous_id } ) {
            $dep_ref = $alltasks->find_id($dep_id);

            # Task depends of a TaskList then find last task from this list
            $dep_ref = $dep_ref->last_subtask if ($dep_ref->is_container());

            push @lst_dep, $dep_ref;
        }
    }

    return \@lst_dep;
}

#create back dependencies (follower tasks) reference list
sub find_follower_lst {
    my $self     = shift;
    my $alltasks = shift;

    my $dep_ref;
    my $dep_id;
    my @lst_dep;
    my $abs_dep;

    if ( $self->get_follower_id ) {
        foreach $dep_id ( @{ $self->get_follower_id } ) {
            $abs_dep = $self->id_to_abs($dep_id);
            $dep_ref = $alltasks->find_id($abs_dep);

            # not the best way but it works
            if ( $dep_ref->is_container() ) {
                $dep_ref = $dep_ref->last_sub_task;
            }
            push @lst_dep, $dep_ref;
        }
    }

    return \@lst_dep;
}

1;
