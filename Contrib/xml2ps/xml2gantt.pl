#!/usr/bin/perl
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!!  This Software is __ALPHA__  !!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# xml2gantt.pl - gantt chart generator for TaskJuggler
#
# Copyright (c) 2001, 2002 by Remo Behn <ray@suse.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# $Id$
#

$| = 1;

use strict;
use warnings;

package tjTask;

    use strict;
    use warnings;
    use Class::MethodMaker
        new_hash_init   => 'new',
        get_set         => [ qw(Index ProjectID complete Priority Type Id Name ParentTask
                                minStart maxStart
                                minEnd maxEnd
                                actualStart actualEnd
                                planStart planEnd
                                h_minStart h_minEnd
                                h_maxStart h_maxEnd
                                h_actualStart h_actualEnd
                                h_planStart h_planEnd
                                x1 y1 x2 y2
                                label label_x label_y ) ],
        struct          => [ qw(Followers Previous Allocations bookedResources) ];

use XML::Parser;
use PostScript::Simple;
use Date::Calc qw(  Today
                    Delta_Days
                    Add_Delta_Days
                    Week_Number
                    Day_of_Week
                    Day_of_Week_Abbreviation
                    Month_to_Text
                    Monday_of_Week);

#-- main package -------------------------------------------------------------
#-- xml parsen und datenstrukturen zusammenrühren
my $t;  # act task
my $w;  # act resource (worker)
my $r;      # act res_name mapping
my %rmap;   # act res_name mapping
my %project;
my @all_tasks;
my @elm_fifo;
my @task_fifo;
my %res_load;
#-- res_load = {
#--     worker1 => {
#--         task1 => [start, end, load],
#--         task2 => [start, end, load],
#--         task3 => [start, end, load]
#--     },
#--     worker2 => {
#--         task1 => [start, end, load],
#--         task3 => [start, end, load]
#--     }
#--     ...
#-- }
my $last_Y_task = 0;

_pars_xml();

#-- postscript stuff
my $task_count      = $#all_tasks+1; #-- wieviele tasks hat das projekt
my $project_name    = $project{'Name'};
$project{'h_start'} =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
my $project_start   = $project{'h_start'};
my ($p_start_year,
    $p_start_month,
    $p_start_day)   = split(/-/, $project_start);
    ($p_start_year, $p_start_month, $p_start_day) = Add_Delta_Days($p_start_year, $p_start_month, $p_start_day, -5);
$project{'h_end'}   =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
my $project_end     = $project{'h_end'};
my ($p_end_year,
    $p_end_month,
    $p_end_day)     = split(/-/, $project_end);
my $project_days    = Delta_Days($p_start_year, $p_start_month, $p_start_day,
                                 $p_end_year, $p_end_month, $p_end_day);
my ($today_year, $today_month, $today_day) = Today();
#-- only for test file ;)
if ( $ARGV[0] eq 'testProject.xml' ) {
    ($today_year, $today_month, $today_day) = (2002, 05, 30);
}
#-- psfile const-settings
my $page_border   = 10;
my $day_x         = 5; #-- day-width
my $task_height   = 5; #-- task-height
my $task_space    = 3; #-- task-space
my $header_height = 10;
my $poster        = '/usr/bin/poster';
my $out_file      = $project{'Id'}.'.eps';
my $out_file_p    = $project{'Id'}.'_poster.eps';
my $res_count     = scalar (keys %rmap);
my $res_height    = (($task_height + $task_space) * $res_count);
#$res_height = 0; # erst mal apschalten
#-- calc page size
my $page_x    = ($page_border * 2) + ($project_days * $day_x);
my $page_y    = ($page_border * 2) +
                $header_height * 3 +
                (($task_height + $task_space) * $task_count) + $res_height;

_make_postscript_file($page_x, $page_y);
_make_poster();

#use Data::Dumper;
#print Data::Dumper->Dump([\%project], ['project']);
#print Data::Dumper->Dump([\@all_tasks], ['all_tasks']);
#print Data::Dumper->Dump([\%res_load], ['res_load']);
#print Data::Dumper->Dump([\%rmap], ['rmap']);


#----------------------------------------------------------------------------------------------------------------
#---------------- make poster ------------------------------
sub _make_poster {
    my $format;
    open(IN, "<$out_file") || die "can't read $out_file\n";
        while(<IN>) {
            next unless /BoundingBox:/;
            $_ =~ s/.*BoundingBox:\s+\d+\s+\d+\s+(\d+)\s+(\d+)/$1\*$2p/;
            chomp;
            $format = $_;
        }
    close(IN);
    if (-f $poster) {
        print "make poster $out_file_p ... ";
            `$poster -i$format -mA4 -p$format $out_file > $out_file_p`;
        print "done\n";
    } else {
        print "INFO: can't make poster, $poster not found !\n";
    }
}

#---------------- xml stuff --------------------------------
sub _pars_xml {
    my $par = new XML::Parser(ErrorContext => 1,
                              Handlers     => { Start   => \&start,
                                                End     => \&end,
                                                Char    => \&text,
                                                Final   => \&final });
    if ( $ARGV[0] ) {
        if ( -r $ARGV[0] ) {
            print "parse xml file: $ARGV[0] ... ";
                $par->parsefile($ARGV[0]);
            print "done\n";
        } else {
            print "can't read $ARGV[0]\n";
            exit 128;
        }
    } else {
        print "no inputfile...\n\n\tUsage: $0 <project.xml>\n\n";
        exit 255;
    }

}

sub start {
    #-- (Expat, Element [, Attr, Val [,...]])
    my $ex  = shift;
    my $element = shift;
    if ( $element eq 'Project' ) { $project{'Id'} = $_[1] }
    if ( $elm_fifo[$#elm_fifo-1] && $elm_fifo[$#elm_fifo-1] eq 'Project' ) {
        $project{'h_start'}   = $_[1] if ( $element eq 'start' );
        $project{'h_end'}     = $_[1] if ( $element eq 'end' );
        $project{'h_now'}     = $_[1] if ( $element eq 'now' );
    }
    push @elm_fifo, $element;
    #-- das erstenal task
    if ( $element eq 'Task' ) {
        $t = tjTask->new(
            ProjectID       => $_[1],
            Followers       => [],
            Previous        => [],
            Allocations     => [],
            bookedResources => []
        );
        push @all_tasks, $t;
    }
    if ( $elm_fifo[$#elm_fifo-1] eq 'Task' ) {
        $t->h_minStart("$_[1]")     if ( $element eq 'minStart' );
        $t->h_minEnd("$_[1]")       if ( $element eq 'minEnd' );
        $t->h_maxStart("$_[1]")     if ( $element eq 'maxStart' );
        $t->h_maxEnd("$_[1]")       if ( $element eq 'maxEnd' );
        $t->h_actualStart("$_[1]")  if ( $element eq 'actualStart' );
        $t->h_actualEnd("$_[1]")    if ( $element eq 'actualEnd' );
        $t->h_planStart("$_[1]")    if ( $element eq 'planStart' );
        $t->h_planEnd("$_[1]")      if ( $element eq 'planEnd' );
        if ($element eq 'Resource') {
            $r = $_[1];
        }
    }
    if ( $elm_fifo[$#elm_fifo] eq 'Allocation' ) {
        $res_load{$_[1]}{$t->Id} = [ $t->h_planStart, $t->h_planEnd ];
        $w = $_[1];
        push @{$t->Allocations}, "$_[1]";
    }
    $t->Id($_[1]) if ( $element eq 'Task' );
}

sub text {
    my ($ex, $string) = @_;
    return if $string =~ /^\s*$/;
    if ( $elm_fifo[$#elm_fifo-1] eq 'Project' ) {
        $project{'Name'}        = $string if ( $elm_fifo[$#elm_fifo] eq 'Name' );
        $project{'Version'}     = $string if ( $elm_fifo[$#elm_fifo] eq 'Version' );
        $project{'Priority'}    = $string if ( $elm_fifo[$#elm_fifo] eq 'Priority' );
        $project{'start'}       = $string if ( $elm_fifo[$#elm_fifo] eq 'start' );
        $project{'end'}         = $string if ( $elm_fifo[$#elm_fifo] eq 'end' );
        $project{'now'}         = $string if ( $elm_fifo[$#elm_fifo] eq 'now' );
    }
    if ( $elm_fifo[$#elm_fifo-1] eq 'Task' ) {
        $t->Index("$string")        if ( $elm_fifo[$#elm_fifo] eq 'Index' );
        $t->Name("$string")         if ( $elm_fifo[$#elm_fifo] eq 'Name' );
        $t->ProjectID("$string")    if ( $elm_fifo[$#elm_fifo] eq 'ProjectID' );
        $t->Priority("$string")     if ( $elm_fifo[$#elm_fifo] eq 'Priority' );
        $t->complete("$string")     if ( $elm_fifo[$#elm_fifo] eq 'complete' );
        $t->Type("$string")         if ( $elm_fifo[$#elm_fifo] eq 'Type' );
        $t->minStart("$string")     if ( $elm_fifo[$#elm_fifo] eq 'minStart' );
        $t->minEnd("$string")       if ( $elm_fifo[$#elm_fifo] eq 'minEnd' );
        $t->maxStart("$string")     if ( $elm_fifo[$#elm_fifo] eq 'maxStart' );
        $t->maxEnd("$string")       if ( $elm_fifo[$#elm_fifo] eq 'maxEnd' );
        $t->actualStart("$string")  if ( $elm_fifo[$#elm_fifo] eq 'actualStart' );
        $t->actualEnd("$string")    if ( $elm_fifo[$#elm_fifo] eq 'actualEnd' );
        $t->planStart("$string")    if ( $elm_fifo[$#elm_fifo] eq 'planStart' );
        $t->planEnd("$string")      if ( $elm_fifo[$#elm_fifo] eq 'planEnd' );
        $t->ParentTask("$string")   if ( $elm_fifo[$#elm_fifo] eq 'ParentTask' );
        push @{$t->Previous}, "$string"         if ( $elm_fifo[$#elm_fifo] eq 'Previous' );
        push @{$t->Followers}, "$string"        if ( $elm_fifo[$#elm_fifo] eq 'Follower' );
        if ( $elm_fifo[$#elm_fifo] eq 'Resource' ) {
            push @{$t->bookedResources}, "$string";
            $rmap{$r} = $string;
        }
    }
    if ( $elm_fifo[$#elm_fifo] eq 'Load' ) {
        push @{$res_load{$w}{$t->Id}}, $string;
    }
}

sub end {
    my ($ex, $element) = @_;
    $#elm_fifo = $#elm_fifo - 1;
}

sub final {
    my ($ex, $string) = @_;
    return;
}

#---------------- bunt stuff --------------------------------
sub _make_postscript_file {
    my ($page_x, $page_y) = @_;
    print "make postscript file: $out_file ...";
    my $p = new PostScript::Simple( xsize   => $page_x,
                                    ysize   => $page_y,
                                    colour  => 1,
                                    units   => "mm" );

      _draw_header($p, $project_name);
      _draw_grid($p);
      _draw_task($p, $page_x, $page_y);
      _draw_res($p);
      _draw_depends($p);
      _draw_label($p);

    $p->output("$out_file");
    print " done\n";
}

sub _draw_depends {
    my $p = shift;
    foreach my $task (@all_tasks) {
        $p->setcolour(0,0,255);
        foreach my $t (@{$task->Previous}) {
            #-- die ende koordinaten vom task holen, von dem ich abhänge
            if ($t) {
                my ($x1, $y1) = __get_start_cood($t);
                #  ---|
                #     |+++
                #  ---|
                $p->line($x1, $y1, $x1+($task_height/2), $y1);
                #  ---|
                #     |---
                #  ---|  +
                #        +
                #        +
                $p->line($x1+($task_height/2), $y1, $x1+($task_height/2), $y1-($task_height/1.5));
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #     +++|
                $p->line($x1+($task_height/2), $y1-($task_height/1.5), $x1-($task_height*2), $y1-($task_height/1.5));
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #   -----|
                #   +
                #   +
                #   +
                $p->line($x1-($task_height*2), $y1-($task_height/1.5), $x1-($task_height*2), $task->y1);
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #   -----|
                #   |
                #   |
                #   |+++++++++++++++++
                $p->line($x1-($task_height*2), $task->y1, $task->x1, $task->y1 );
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #   -----|
                #   |
                #   |
                #   |----------------->
                #                    -^- auf deutsch == pfeil zeichnen ;)
                $p->polygon($task->x1-2, $task->y1-1,
                            $task->x1,$task->y1,
                            $task->x1,$task->y1,
                            $task->x1-2, $task->y1+1,
                            $task->x1-2, $task->y1-1,   1);
            }
        }
    }
}

#-- holt die ende-koordinaten von einem task anhand der Id
sub __get_start_cood {
    my $id = shift;
    my ($x, $y);
    foreach my $t (@all_tasks) {
        return ($t->x2, $t->y2) if ( $t->Id eq $id );
    }
}

#-- die task-decr draufmalen, ganz zum schluß um alles zu überpinseln
sub _draw_label {
    my $p = shift;
    $p->setcolour(0,0,0);
    foreach my $task (@all_tasks) {
        print "+";
        $p->setfont("Helvetica", 8) if ( $task->Type eq 'Task' );
        $p->setfont("Helvetica", 6) if ( $task->Type eq 'Milestone' );
        $p->text($task->label_x, $task->label_y, $task->label);
    }
}

#-- die res malen
sub _draw_res {
    my $p = shift;
    my $c = 0;
    my $a = 0;
    foreach my $i (sort keys %res_load) {
        foreach my $ii (keys %{$res_load{$i}}) {
            my $res = $rmap{$i};
            my $taskID = $ii;
            my ($start, $end, $load) = @{$res_load{$i}{$ii}};
                $start =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
                $end =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
            #-- wieviele tage vom anfang her fängt der task an
            my ($start_year, $start_month, $start_day)  = split(/-/, $start);
            my ($end_year, $end_month, $end_day)        = split(/-/, $end);
               ($end_year, $end_month, $end_day) = Add_Delta_Days($end_year, $end_month, $end_day, 1);
            my $start_delta = Delta_Days(   $p_start_year, $p_start_month, $p_start_day,
                                            $start_year, $start_month, $start_day);
            #-- länge des tasks in tagen
            my $task_length = Delta_Days(   $start_year, $start_month, $start_day,
                                            $end_year, $end_month, $end_day);
            #-- balken koordinaten
            my $_x1 = $start_delta * $day_x;
            my $_y1 = $last_Y_task + ( ($task_height + $task_space) * $c ) + ($header_height/2);
            my ($x1, $y1) = _trans_coord($_x1, $_y1);
            my $_x2 = $_x1 + ($task_length * $day_x);
            my $_y2 = $_y1 + $task_height;
            my ($x2, $y2) = _trans_coord($_x2, $_y2);
            #-- balken
            $p->setcolour(222,222,222);
            $p->box($x1, $y1, $x2, $y2, 1);
            #-- rahmen drum
            $p->setcolour(0,0,0);
            $p->box($x1, $y1, $x2, $y2, 0);
            #-- linie dazwischen
            my $l_y = $y2+($task_space/2)-($task_height/2);
            $p->line($page_border+5, $l_y, $page_x-($page_border*2), $l_y );
            #-- res-name
            $p->setfont("Helvetica", 8);
            $p->text($page_border+5, $y1-($task_height/1.5), "$res");
            #-- load reinschreiben
            if ($task_length >= 2) {
                $p->text($x2-6, $y1-($task_height/1.5), "$load");
            }
            $a++;
        }
        $c++;
        $a = 0;
    }
}

#-- die tasks malen
sub _draw_task {
    my ($p, $page_x, $page_y) = @_;
    foreach my $task (@all_tasks) {
        print ".";
        my $nr      = $task->Index;
        my $name    = $task->Name;
        my $start   = $task->h_planStart;
            $start =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
        my $end     = $task->h_planEnd;
            $end =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
        my $persent = $task->complete;
            $persent = 0 if $persent <= 0;
        #-- wieviele tage vom anfang her fängt der task an
        my ($start_year, $start_month, $start_day)  = split(/-/, $start);
        my ($end_year, $end_month, $end_day)        = split(/-/, $end);
        if ( ($task->Type eq 'Task') || ($task->Type eq 'Container') ) {
           ($end_year, $end_month, $end_day) = Add_Delta_Days($end_year, $end_month, $end_day, 1);
        }
        my $start_delta = Delta_Days(   $p_start_year, $p_start_month, $p_start_day,
                                        $start_year, $start_month, $start_day);
        #-- länge des tasks in tagen
        my $task_length = Delta_Days(   $start_year, $start_month, $start_day,
                                        $end_year, $end_month, $end_day);
        #-- balken koordinaten
        my $_x1 = $start_delta * $day_x;
        my $_y1 = ($task_height * $nr ) + ($task_space * $nr) + 10; # 10 = nicht über die
                                                                    # monatsnamen oder wochennummern malen ;)
        my ($x1, $y1) = _trans_coord($_x1, $_y1);
        my $_x2 = $_x1 + ($task_length * $day_x);
        my $_y2 = $_y1 + $task_height;
        my ($x2, $y2) = _trans_coord($_x2, $_y2);
        #-- letzte y-koordinate mweken um nachher noch die res-balken zu malen
        $last_Y_task =  $_y2 if ($_y2 > $last_Y_task);
        #-- die koordinaten für anfang und ende des tasks merken, da fangen die
        #-- depend-lines an oder da gehen sie halt hin, hoffentlich ;)
        $task->x1($x1); $task->y1($y1-($task_height/2));
        $task->x2($x2); $task->y2($y2+($task_height/2));
        if ( $task->Type eq 'Task' ) {
            #-- länge von % feritg balken
            my ($per_length, $d) = _trans_coord($_x1 + (($task_length/100*$persent) * $day_x), 0);
            #-- wenn das ende vor heute liegt und der task nicht 100% fertig hat, dann rot
            if ( Delta_Days($today_year, $today_month, $today_day, $end_year, $end_month, $end_day) < 0 ) {
                if ( $persent < 100 ) {
                    $p->setcolour(255,0,0);
                    $p->box($x1, $y1, $x2, $y2, 1);
                }
            } else {
                $p->setcolour(255,255,255);
                $p->box($x1, $y1, $x2, $y2, 1);
            }
            #-- % done balken oben drüber pinseln
            if ($persent > 0) {
                $p->setcolour(0,255,0);
                $p->box($x1, $y1, $per_length, $y2, 1);
            }
            #-- rahmen um den task
            $p->setcolour(0,0,0);
            $p->box($x1, $y1, $x2, $y2, 0);
            #-- text
            $task->label($name);
            $task->label_x($x1+1);
            $task->label_y($y1-($task_height/1.5));
        }
        if ( $task->Type eq 'Container' ) {
            $p->setcolour(0,0,0);
            $p->box($x1-1, $y1, $x2+1, $y2+($task_height/1.2), 1);
            #-- pfeil vorn
            $p->polygon(
                $x1+1, $y2+($task_height/1.2),
                $x1-1, $y2+($task_height/1.2),
                $x1, $y2+($task_height/2),
                $x1+1, $y2+($task_height/1.2),
                $x1+1, $y2+($task_height/1.2), 1);
            #-- pfeil hinten
            $p->polygon(
                $x2+1, $y2+($task_height/1.2),
                $x2-1, $y2+($task_height/1.2),
                $x2, $y2+($task_height/2),
                $x2+1, $y2+($task_height/1.2),
                $x2+1, $y2+($task_height/1.2), 1);
            #-- text
            $task->label($name);
            $task->label_x($x1+1);
            $task->label_y($y1-($task_height/1.5));
        }
        if ( $task->Type eq 'Milestone' ) {
            $p->setcolour(0,0,0);
            my ($x, $y) = _trans_coord($_x1, $_y1+($task_height/2));
            $p->circle($x, $y, 1, 1);
            $p->setfont("Helvetica", 6);
            my $am = sprintf('%02d', $start_month);
            my $ad = sprintf('%02d', $start_day);
            #-- text
            $task->label("$name ($am-$ad)");
            $task->label_x($x1+2);
            $task->label_y($y1-($task_height/2));
        }
    }
}

#-- monate, wocjen, tage usw. malen
sub _draw_grid {
    my $p = shift;
    #-- wenn die project-laufzeit 21 tage über das ende des letzten tasks hinausgeht
    #-- meckern wir mal rum ;)))
    my $last_task = $all_tasks[$#all_tasks];
    my $last_task_end = $last_task->h_planEnd;
        $last_task_end =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
    my ($l_end_year,
        $l_end_month,
        $l_end_day)   = split(/-/, $last_task_end);
    my $delta = Delta_Days($l_end_year, $l_end_month, $l_end_day,
                           $p_end_year, $p_end_month, $p_end_day);
    if ( $delta  > 21 ) {
        my ($l_end_year, $l_end_month, $l_end_day) = Add_Delta_Days($l_end_year, $l_end_month, $l_end_day, 20);
        print "\n\tproject-end  : $project_end\n\tlast task-end: $last_task_end\n\tdelta is $delta days\n\tplease use $l_end_year-$l_end_month-$l_end_day as end of project for nice graph drawing ;)))\n";
    }
    #-- wenn der project-start 7 tage vor dem des ersten tasks liegt meckern wir mal rum ;)))
    my $first_task = $all_tasks[0];
    my $first_task_start = $first_task->h_planStart;
        $first_task_start =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
    my ($f_start_year,
        $f_start_month,
        $f_start_day)   = split(/-/, $first_task_start);
    my $first_delta = Delta_Days($p_start_year, $p_start_month, $p_start_day,
                                 $f_start_year, $f_start_month, $f_start_day);
    if ( $first_delta  > 13 ) { # 13 weil ich 5 tage vorher das zeichnen anfange, und 7+5 is nu ma 13
        my ($l_end_year, $l_end_month, $l_end_day) = Add_Delta_Days($f_start_year, $f_start_month, $f_start_day, -7);
        print "\n\tproject-start  : $project_start\n\tfirst task-start: $first_task_start\n\tdelta is $first_delta days\n\tplease use $l_end_year-$l_end_month-$l_end_day as start of project for nice graph drawing ;)))\n";
    }
    for(my $i = 0; $i <= $project_days; $i++) {
        #-- bei welchem tag sind wir gerade
        my ($act_year, $act_month, $act_day) = Add_Delta_Days($p_start_year, $p_start_month, $p_start_day, $i);
        #-- welcher woche ist das
        my $act_week = Week_Number($act_year, $act_month, $act_day);
        #-- welcher tag der woche ist das
        my $act_dow  = Day_of_Week($act_year, $act_month, $act_day);
        #-- welcher monat (name)
        my $act_month_name = Month_to_Text($act_month);
        #-- welcher tag (name)
        my $act_day_name = Day_of_Week_Abbreviation($act_dow);
        #-- farbe für die tage festlegen
        $p->setcolour('white');
        #-- ist es ein wochenende ?
        if ( $act_day_name eq 'Sat' || $act_day_name eq 'Sun' ) {
            $p->setcolour(250,250,165); # sand
        }
        #-- heute wird auch anders angezeigt
        if ( Delta_Days(  $act_year, $act_month, $act_day,
                          $today_year, $today_month, $today_day) == 0 ) {
            $p->setcolour(191,191,191); # lgray
        }

        my $h_month_week = 3;
        my ($_x, $_y)   = ($i*$day_x, $day_x*$h_month_week);
        my ($x, $y)     = _trans_coord($_x, $_y);
        $p->box($x, $y, $x+$day_x, $page_border, 1);
        #-- die linien haben unterschiedliche höhe
        #-- is es ein wochen-anfang
        if ( $act_dow == 1 ) {
            $h_month_week = 2;
        }
        #-- is es ein monatserster
        if ( $act_day == 1 || $act_day == 01) {
            $h_month_week = 1;
        }
        ($_x, $_y)   = ($i*$day_x, $day_x*$h_month_week);
        ($x, $y)     = _trans_coord($_x, $_y);
        $p->line($x, $y, $x, $page_border, 191, 191, 191);
        #-- wochen-nummer reinpinseln
        if ( $h_month_week == 2 ) {
            $p->setcolour(0,0,0);
            $p->setfont("Times-Roman", 10);
            $p->text($x+1, $y-($header_height/$h_month_week)+($header_height/4), $act_week);
        }
        #-- monats-namen reinpinseln
        if ( $h_month_week == 1 || $i == 0 ) {
            $p->setcolour(0,0,0);
            $p->setfont("Times-Roman", 12);
            $p->text($x+1, $y-($header_height/$h_month_week)+($header_height/1.5), "$act_month_name $act_year");
            #-- wenn der erste im monat ein wochenanfang ist, auch die wochen-nummer reinschreiben
            if ( $act_dow == 1 ) {
                $p->setfont("Helvetica", 10);
                $p->text($x+1, $y-($header_height/$h_month_week)+($header_height/4), $act_week);
            }
        }
        #-- tage in den header schreiben
        $p->setcolour(0,0,0);
        $p->setfont("Helvetica", 6);
        $p->text($x+0.5, $page_y-$page_border-($header_height*3)+($task_height/1.5), sprintf('%02d', $act_day));
    }
}

#-- überschrift mit project-name zeichnen
sub _draw_header {
    my ($p, $str) = @_;
    $p->setcolour(0,0,0);
    $p->setfont("Helvetica", 18);
    my ($x, $y) = _trans_coord(0,0);
    $p->text($x, $y+($header_height/2), $str);
    $p->setcolour(255,255,255);
}

#-- coords umrechnen
sub _trans_coord {
    my ($draw_x, $draw_y) = @_;

    my $ps_x = $page_border + $draw_x;
    my $ps_y = $page_y - $page_border - $draw_y - $header_height;

    return ($ps_x, $ps_y);
}

__END__

 |------------------------------------------------------------------------|
 |            page-border                                                 |
 |   |----------------------------------------------------------------|   |
 |   |        header                                                  |   |
 |   |----------------------------------------------------------------|   |
 |   |0,0                                                             |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   | Y
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |                                                                |   |
 |   |----------------------------------------------------------------|   |
 |                                                                        |
 |------------------------------------------------------------------------|
0,0                                  X


