#!/usr/bin/perl
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!!  This Software is __ALPHA__  !!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# xml2gantt_png.pl - gantt chart generator for TaskJuggler
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
use GD;
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
my $t;
my %project;
my @all_tasks;
my @elm_fifo;
my @task_fifo;

_pars_xml();

#-- png stuff
my $task_count      = $#all_tasks+1; #-- wieviele tasks hat das projekt
my $project_name    = $project{'Name'};
$project{'h_start'} =~ s/(\d\d\d\d-\d\d-\d\d) .*/$1/g;
my $project_start   = $project{'h_start'};
my ($p_start_year,
    $p_start_month,
    $p_start_day)   = split(/-/, $project_start);
    $p_start_day    = 01;
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
#-- file const-settings
my $page_border   = 50;
my $day_x         = 10; #-- day-width
my $task_height   = 15; #-- task-height
my $task_space    = 15; #-- task-space
my $header_height = 35;
my $out_file      = $project{'Id'}.'.png';
#-- calc page size
my $page_x    = ($page_border * 2) + ($project_days * $day_x);
my $page_y    = ($page_border * 2) + $header_height*3 + (($task_height + $task_space) * $task_count);


print "make png file: $out_file ...";
    my $p = new GD::Image($page_x, $page_y);
        $p->interlaced('true');
        my $white   = $p->colorAllocate(255,255,255);
        my $black   = $p->colorAllocate(0,0,0);

        my $lred    = $p->colorAllocate(255,0,0);
        my $red     = $p->colorAllocate(191,0,0);
        my $dred    = $p->colorAllocate(127,0,0);

        my $lgreen  = $p->colorAllocate(0,255,0);
        my $green   = $p->colorAllocate(0,191,0);
        my $dgreen  = $p->colorAllocate(0,127,0);

        my $lblue   = $p->colorAllocate(0,0,255);
        my $blue    = $p->colorAllocate(0,0,191);
        my $dblue   = $p->colorAllocate(0,0,127);

        my $lgray   = $p->colorAllocate(191,191,191);
        my $gray    = $p->colorAllocate(127,127,127);
        my $dgray   = $p->colorAllocate(63,63,63);

        my $sand    = $p->colorAllocate(250,250,165);
        my $lyellow = $p->colorAllocate(255,255,0);
        my $yellow  = $p->colorAllocate(191,191,0);
        my $dyellow = $p->colorAllocate(127,127,0);
    _make_png_file($page_x, $page_y);
print " done\n";

#use Data::Dumper;
#print Data::Dumper->Dump([\%project], ['project']);
#print Data::Dumper->Dump([\@all_tasks], ['all_tasks']);

#----------------------------------------------------------------------------------------------------------------
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
        push @{$t->Allocations}, "$_[1]"    if ( $elm_fifo[$#elm_fifo] eq 'Allocation' );
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
        push @{$t->bookedResources}, "$string"  if ( $elm_fifo[$#elm_fifo] eq 'Resource' );
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
sub _make_png_file {
    my ($page_x, $page_y) = @_;

      _draw_header($p, $project_name);
      _draw_grid($p);
      _draw_task($p, $page_x, $page_y);
      _draw_depends($p);
      _draw_label($p);

    my $png_image = $p->png;
    open(OUT, ">$out_file") || die "Oops...\n";
        binmode OUT;
        print OUT $png_image;
    close(OUT);
}

sub _draw_depends {
    my $p = shift;
    foreach my $task (@all_tasks) {
        foreach my $t (@{$task->Previous}) {
            #-- die ende koordinaten vom task holen, von dem ich abhänge
            if ($t) {
                my ($x1, $y1) = __get_start_cood($t);
                #  ---|
                #     |+++
                #  ---|
                $p->line($x1, $y1-$task_height, $x1+($task_height/2), $y1-$task_height, $blue);
                #  ---|
                #     |---
                #  ---|  +
                #        +
                #        +
                $p->line($x1+($task_height/2), $y1-$task_height, $x1+($task_height/2), $y1+($task_height/8), $blue);
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #     +++|
                $p->line($x1+($task_height/2), $y1+($task_height/8), $x1-$task_height, $y1+($task_height/8), $blue);
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #   -----|
                #   +
                #   +
                #   +
                $p->line($x1-$task_height, $y1+($task_height/8), $x1-$task_height, $task->y1+$task_height, $blue);
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #   -----|
                #   |
                #   |
                #   |+++++++++++++++++
                $p->line($x1-$task_height, $task->y1+$task_height, $task->x1, $task->y1+$task_height, $blue);
                #  ---|
                #     |---
                #  ---|  |
                #        |
                #   -----|
                #   |
                #   |
                #   |-----------------*
                #                    -^- auf deutsch == der kreis ;)
                $p->arc($task->x1, $task->y1+$task_height, $task_height/2, $task_height/2, 0, 360, $black);
                $p->fillToBorder($task->x1, $task->y1+$task_height, $black, $black);
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
    foreach my $task (@all_tasks) {
        print "+";
        my $x = $task->label_x;
        my $y = $task->label_y;
        if ( $task->Type eq 'Task' ) {
            $x = $x + 10; $y = $y + ($task_height / 1.5);
        }
        if ( $task->Type eq 'Container' ) {
            $x = $x + $task_height; $y = $y - ($task_height / 2);
        }
        $p->string(gdSmallFont,$x, $y, $task->label, $black);
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
        my $x1 = $start_delta * $day_x;
        my $y1 = ($task_height * $nr ) + ($task_space * $nr) + ($header_height*2);
        my $x2 = $x1 + ($task_length * $day_x);
        my $y2 = $y1 + $task_height;
        #-- die koordinaten für anfang und ende des tasks merken, da fangen die
        #-- depend-lines an oder da gehen sie halt hin, hoffentlich ;)
        $task->x1($x1); $task->y1($y1-($task_height/2));
        $task->x2($x2); $task->y2($y2+($task_height/2));
        if ( $task->Type eq 'Task' ) {
            #-- länge von % feritg balken
            my ($per_length, $d) = ($x1 + (($task_length/100*$persent) * $day_x), 0);
            #-- wenn das ende vor heute liegt und der task nicht 100% fertig hat, dann rot
            if ( Delta_Days($today_year, $today_month, $today_day, $end_year, $end_month, $end_day) < 0 ) {
                if ( $persent < 100 ) {
                    $p->filledRectangle($x1, $y1, $x2, $y2, $red);
                }
            } else {
                $p->filledRectangle($x1, $y1, $x2, $y2, $white);
            }
            #-- % done balken oben drüber pinseln
            if ($persent > 0) {
                $p->filledRectangle($x1, $y1, $per_length, $y2, $green);
            }
            #-- rahmen um den task
            $p->rectangle($x1, $y1, $x2, $y2, $black);
            #-- text merken
            $task->label($name);
            $task->label_x($x1+1);
            $task->label_y($y1-($task_height/1.5));
        }
        if ( $task->Type eq 'Container' ) {
            $p->filledRectangle($x1-($day_x/2), $y1, $x2+($day_x/2), $y2-($task_height/2), $black);
#--            #-- pfeil vorn
#--            my $poly = new GD::Polygon;
#--                #  *++
#--                #
#--                #   +
#--                $poly->addPt($x1-($day_x/2), $y1);
#--                #  ++*
#--                #
#--                #   +
#--                $poly->addPt($x1+($day_x/2), $y1);
#--                #  +++
#--                #
#--                #   *
#--                $poly->addPt($x1, $y2);
#--            $p->filledPolygon($poly, $black);
#--            #-- pfeil hinten
#--            my $poly1 = new GD::Polygon;
#--                #  *++
#--                #
#--                #   +
#--                $poly1->addPt($x2-($day_x/2), $y2-($task_height/2));
#--                #  ++*
#--                #
#--                #   +
#--                $poly1->addPt($x2+($day_x/2), $y2-($task_height/2));
#--                #  +++
#--                #
#--                #   *
#--                $poly1->addPt($x2, $y2);
#--            $p->filledPolygon($poly1, $black);
            #-- text merken
            $task->label($name);
            $task->label_x($x1+1);
            $task->label_y($y1-($task_height/1.5));
        }
        if ( $task->Type eq 'Milestone' ) {
            my ($x, $y) = ($x1, $y1+($task_height/2));
            $p->arc($x, $y, $task_height/2, $task_height/2, 0, 360, $black);
            $p->fillToBorder($x, $y, $black, $black);
            my $am = sprintf('%02d', $start_month);
            my $ad = sprintf('%02d', $start_day);
            #-- text merken
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
        my $__color = $white;
        #-- ist es ein wochenende ?
        if ( $act_day_name eq 'Sat' || $act_day_name eq 'Sun' ) {
            $__color = $sand;
        }
        #-- heute wird auch anders angezeigt
        if ( Delta_Days(  $act_year, $act_month, $act_day,
                          $today_year, $today_month, $today_day) == 0 ) {
            $__color = $lgray;
        }

        my $h_month_week = 3;
        my ($x, $y)   = ($i*$day_x, $day_x*$h_month_week+$header_height);
        $p->filledRectangle($x,$y, $x+$day_x, $page_y-$page_border, $__color);
        #-- die linien haben unterschiedliche höhe
        #-- is es ein wochen-anfang
        if ( $act_dow == 1 ) {
            $h_month_week = 2;
        }
        #-- is es ein monatserster
        if ( $act_day == 1 || $act_day == 01) {
            $h_month_week = 1;
        }
        ($x, $y)   = ($i*$day_x, $day_x*$h_month_week);
        $p->line($x, $y+$header_height, $x, $page_y-$page_border, $lgray);
        #-- wochen-nummer reinpinseln
        if ( $h_month_week == 2 ) {
            $p->string(gdTinyFont,$x+3, $y+$header_height, $act_week, $black);
        }
        #-- monats-namen reinpinseln
        if ( $h_month_week == 1 || $i == 0 ) {
            $p->string(gdTinyFont,$x+3, $y+$header_height, "$act_month_name $act_year", $black);
            #-- wenn der erste im monat ein wochenanfang ist, auch die wochen-nummer reinschreiben
#            if ( $act_dow == 1 ) {
#                $p->string(gdTinyFont,$x+3, ($y+$header_height), $act_week, $black);
#            }
        }
        #-- tage in den header schreiben
        #$p->string(gdTinyFont, $x+1, $y, sprintf('%02d', $act_day), $black);
    }
}

#-- überschrift mit project-name zeichnen
sub _draw_header {
    my ($p, $str) = @_;
    $p->string(gdLargeFont,10, 0+($header_height/2), $str, $black);
}


