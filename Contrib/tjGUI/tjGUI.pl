#!/usr/bin/perl
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
# !!!  This Software is __ALPHA__  !!!
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# tjGUI.pl - Perl-Tk GUI for TaskJuggler
#
# Copyright (c) 2001, 2002 by Remo Behn <ray@suse.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# $Id$
#

use strict;
use warnings;
use tjTask;
use XML::Parser;
use Tk;
use Tk::HList;
use Tk::Tiler;
use Date::Calc qw(  Today
                    Delta_Days
                    Add_Delta_Days
                    Week_Number
                    Day_of_Week
                    Day_of_Week_Abbreviation
                    Month_to_Text
                    Monday_of_Week);

#-- global vars for xml-stuff
my $t;      # act task
my $w;      # act resource (worker)
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

#-- global vars
my %hlist_entrys;       #-- $hlist_entrys{taskID} = entry_ref

#-- bunt
my $top = MainWindow->new();
    $top->geometry("650x500");

    #-- buttons oben
    my $f_head = $top->Frame(   -relief => 'flat',
                                -border => 1 )->pack(   -padx   => 3,
                                                        -pady   => 0,
                                                        -fill   => 'x');

        my $b_Quit  = $f_head->Button(  -text       => 'quit',
                                        -relief     => 'groove',
                                        -command    => sub { $top->destroy }
                                        )->pack( -side => 'left');
        my $b_Bunt  = $f_head->Button(  -text       => 'gantt chart',
                                        -relief     => 'groove',
                                        -command    => sub { &_gantt }
                                        )->pack( -side => 'left');

    #-- working bereich
    my $f_top = $top->Frame(    -relief => 'sunken',
                                -border => 1 )->pack(   -padx   => 3,
                                                        -pady   => 3,
                                                        -expand => 'yes',
                                                        -fill   => 'both');

        my $task_list = $f_top->Scrolled(qw\HList   -separator /
                                                    -selectmode extended
                                                    -width      30
                                                    -height     20
                                                    -indent     35
                                                    -scrollbars se
                                                    -itemtype   imagetext \
                                            )->pack(    -side   => 'left',
                                                        -fill   => 'y');
            $task_list->configure( -command => sub { _display_task_data($task_list->info('data', $_[0])) } );

        my $work_area_frame = $f_top->Frame(    -relief => 'flat',
                                                -border => 0)->pack (   -padx   => 0,
                                                                        -pady   => 0,
                                                                        -expand => 'yes',
                                                                        -fill   => 'both');

            #-- beim start bissle geschwafel reinschreiben
            my $startUpTextFrame = $work_area_frame->Frame()->pack();
                $startUpTextFrame->Label( -text => 'Welcome to the TaskJuggler-GUI' )->pack();
                $startUpTextFrame->Label( -text => 'version 0.1' )->pack();
                $startUpTextFrame->Label( -text => 'Copyright (c) 2002 by Remo Behn <ray@suse.de>' )->pack( -pady => 20);
                $startUpTextFrame->Label( -text => 'This program is free software; you can redistribute it and/or modify' )->pack();
                $startUpTextFrame->Label( -text => 'it under the terms of version 2 of the GNU General Public License as' )->pack( -padx => 5 );
                $startUpTextFrame->Label( -text => 'published by the Free Software Foundation.' )->pack();

    #-- statusline
    my $f_bottom = $top->Frame( -relief => 'flat',
                                -border => 1 )->pack(   -padx   => 3,
                                                        -pady   => 0,
                                                        -fill   => 'x');
        my $status_line = $f_bottom->Label( -text => 'have a nice day ...'
                                            )->pack( -side => 'left', -padx => 3);


_pars_xml();

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
my $task_count    = $#all_tasks+1; #-- wieviele tasks hat das projekt
my $page_border   = 10;
my $header_height = 35;
my $day_x         = 20; #-- day-width
my $task_height   = 15; #-- task-height
my $task_space    = 5; #-- task-space
my $res_count     = scalar (keys %rmap);
my $res_height    = (($task_height + $task_space) * $res_count);

#-- calc page size
my $page_x    = ($page_border*2) + ($project_days * $day_x) + ($page_border*2);
my $page_y    = ($page_border * 2) +
                ($header_height * 3) +
                (($task_height + $task_space) * $task_count) + $res_height;

MainLoop;

#-----------------------------------------------------------------------------
sub _gantt {
    my ($x, $y) = ($page_x, $page_y);

    foreach ($work_area_frame->children) { $_->destroy }
    my $c = $work_area_frame->Scrolled(qw/Canvas
                                            -width          60
                                            -height         45
                                            -relief         flat
                                            -borderwidth    5
                                            -scrollbars     se
                                            -scrollregion/  => ['0', '0', $x, $y])->pack( -expand => 'yes', -fill => 'both');
    _draw_grid($c, $page_x, $page_y);
}

sub _draw_grid {
    my ($c, $page_x, $page_y) = @_;
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
        my $color = 'white';
        #-- ist es ein wochenende ?
        if ( $act_day_name eq 'Sat' || $act_day_name eq 'Sun' ) {
            $color = 'beige'; #-- or LightGoldenrodYellow
        }
        #-- heute wird auch anders angezeigt
        if ( Delta_Days(  $act_year, $act_month, $act_day,
                          $today_year, $today_month, $today_day) == 0 ) {
            $color = 'gray75';
        }

        my $h_month_week = 3;
        my ($x, $y) = ($i*$day_x+$page_border, $day_x*$h_month_week);
        $c->createRectangle($x, $y, $x+$day_x, $page_y-$page_border, -outline => $color, -fill => $color);
        #-- die linien haben unterschiedliche höhe
        #-- is es ein wochen-anfang
        if ( $act_dow == 1 ) {
            $h_month_week = 2;
        }
        #-- is es ein monatserster
        if ( $act_day == 1 || $act_day == 01) {
            $h_month_week = 1;
        }
        if ( $h_month_week == 3 ) {
            ($x, $y)   = ($i*$day_x+$page_border, $day_x*$h_month_week);
            $c->createLine($x, $y, $x, $page_y-$page_border, -fill => 'gray75');
        }
        #-- tage in den header schreiben
        ($x, $y)   = ($i*$day_x+$page_border, $day_x*3.5);
        $c->createText($x+3, $y, -text => sprintf('%02d', $act_day), -anchor => 'w');
        #-- wochen-nummer reinpinseln
        if ( $h_month_week == 2 ) {
            ($x, $y)   = ($i*$day_x+$page_border, $header_height+($day_x/2));
            $c->createText($x+3, $y, -text => sprintf('%02d', $act_week), -anchor => 'w');
            ($x, $y)   = ($i*$day_x+$page_border, $day_x*$h_month_week);
            $c->createLine($x, $y, $x, $page_y-$page_border, -fill => 'black');
        }
        #-- monats-namen reinpinseln
        if ( $h_month_week == 1 || $i == 0 ) {
            ($x, $y)   = ($i*$day_x+$page_border, $header_height-($day_x/2));
            $c->createText($x+3, $y, -text => "$act_month_name $act_year", -anchor => 'w');
            ($x, $y)   = ($i*$day_x+$page_border, $day_x*$h_month_week);
            $c->createLine($x, $y, $x, $page_y-$page_border, -fill => 'black');
            ##-- wenn der erste im monat ein wochenanfang ist, auch die wochen-nummer reinschreiben
            if ( $act_dow == 1 ) {
                ($x, $y)   = ($i*$day_x+$page_border, $header_height+($day_x/2));
                $c->createText($x+3, $y, -text => sprintf('%02d', $act_week), -anchor => 'w');
            }
        }
    }
}

sub _display_task_data {
    my $t = shift;
    foreach ($work_area_frame->children) { $_->destroy }
    my $l = $work_area_frame->Frame()->pack( -side => 'left', -fill => 'y' );
    my $v = $work_area_frame->Frame()->pack( -side => 'left', -fill => 'y' );
        my ($y, $m, $d, $h, $mi, $s);
        if ( $t->planStart =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->planStart);
            $t->planStart( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->planEnd =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->planEnd);
            $t->planEnd( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->minStart =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->minStart);
            $t->minStart( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->minEnd =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->minEnd);
            $t->minEnd( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->maxStart =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->maxStart);
            $t->maxStart( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->maxEnd =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->maxEnd);
            $t->maxEnd( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->actualStart =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->actualStart);
            $t->actualStart( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }
        if ( $t->actualEnd =~ /^\d+$/ ) {
            ($y, $m, $d, $h, $mi, $s) = Date::Calc::Time_to_Date($t->actualEnd);
            $t->actualEnd( sprintf("%04d-%02d-%02d %02d:%02d:%02d" ,$y, $m, $d, $h, $mi, $s) );
        }

        $l->Label( -text => 'short descr.:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->Name )->pack( -anchor => 'w' );
        $l->Label( -text => ' ' )->pack( -anchor => 'w', -padx => 15 );
            $v->Label( -text => '-'x15 )->pack( -anchor => 'w' );

        $l->Label( -text => 'id:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->Id )->pack( -anchor => 'w' );
        $l->Label( -text => 'type:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->Type )->pack( -anchor => 'w' );
        $l->Label( -text => ' ' )->pack( -anchor => 'w', -padx => 15 );
            $v->Label( -text => '-'x15 )->pack( -anchor => 'w' );

        $l->Label( -text => 'plan date:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->planStart." ---> ".$t->planEnd )->pack( -anchor => 'w' );
        $l->Label( -text => 'min date:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->minStart." ---> ".$t->minEnd )->pack( -anchor => 'w' );
        $l->Label( -text => 'max date:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->maxStart." ---> ".$t->maxEnd )->pack( -anchor => 'w' );
        $l->Label( -text => 'actual date:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->actualStart." ---> ".$t->actualEnd )->pack( -anchor => 'w' );
        $l->Label( -text => ' ' )->pack( -anchor => 'w', -padx => 15 );
            $v->Label( -text => '-'x15 )->pack( -anchor => 'w' );

        $l->Label( -text => 'start buffer:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->startBuffer." %" )->pack( -anchor => 'w' );
        $l->Label( -text => 'end buffer:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->endBuffer." %" )->pack( -anchor => 'w' );
        $l->Label( -text => 'complete:' )->pack( -anchor => 'w', -padx => 15  );
            $v->Label( -text => $t->complete." %" )->pack( -anchor => 'w' );
        $l->Label( -text => ' ' )->pack( -anchor => 'w', -padx => 15 );
            $v->Label( -text => '-'x15 )->pack( -anchor => 'w' );

        $l->Label( -text => 'resources' )->pack( -anchor => 'w', -padx => 15 );
        my $all_c = 0;
        foreach my $all ( @{$t->bookedResources} ) {
            $l->Label( -text => '' )->pack( -anchor => 'w', -padx => 15 ) if ($all_c > 0);
                $v->Label( -text => $all )->pack( -anchor => 'w' );
            $all_c++;
        }
}

# taskliste durchrühren und in die hlist einfügen
sub _fill_hlist {
    my $top = shift;
    my $task_img    = $top->Bitmap(-file => 'task.xbm');
    my $mtask_img   = $top->Bitmap(-file => 'mtask.xbm');
    my $mile_img    = $top->Bitmap(-file => 'mile.xbm');

    my $last_container = $task_list;
    my $c = 1;
    foreach my $t (@all_tasks) {
        if ( $t->Type eq 'Container' ) {
            my $e = $top->add($top.$c, -text => $t->Name, -image => $mtask_img, -data => $t);
            $hlist_entrys{$t->Id} = $e;
            $last_container = $e;
        }
        if ( $t->Type eq 'Task' ) {
            my $e = $top->addchild($last_container, -text => $t->Name, -image => $task_img, -data => $t);
            $hlist_entrys{$t->Id} = $e;
        }
        if ( $t->Type eq 'Milestone' ) {
            my $e = $top->addchild($last_container, -text => $t->Name, -image => $mile_img, -data => $t);
            $hlist_entrys{$t->Id} = $e;
        }
        $c++;
    }
}

#------------------------- XML-STUFF -----------------------------------------
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
            startBuffer     => 0,
            endBuffer       => 0,
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
        $t->startBuffer("$string")  if ( $elm_fifo[$#elm_fifo] eq 'startBufferSize' );
        $t->endBuffer("$string")    if ( $elm_fifo[$#elm_fifo] eq 'EndBufferSize' );
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
    _fill_hlist($task_list);
}

