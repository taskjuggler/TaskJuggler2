package tjTask;

use strict;
use warnings;
use Class::MethodMaker
    new_hash_init   => 'new',
    get_set         => [ qw(Index
                            ProjectID
                            complete
                            Priority
                            Type
                            Id
                            Name
                            ParentTask
                            startBuffer     endBuffer
                            minStart        maxStart
                            minEnd          maxEnd
                            actualStart     actualEnd
                            planStart       planEnd
                            h_minStart      h_minEnd
                            h_maxStart      h_maxEnd
                            h_actualStart   h_actualEnd
                            h_planStart     h_planEnd
                            ) ],
    struct          => [ qw(Followers Previous Allocations bookedResources) ];

1;
