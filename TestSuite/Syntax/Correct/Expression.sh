#! /bin/sh

cmp Expression_containstask.html Expression_containstask_ref.html
if test $? -ne 0 ; then
  exit 1
fi

cmp Expression_issubtaskof.html Expression_issubtaskof_ref.html
if test $? -ne 0 ; then
  exit 1
fi

cmp Expression_istask.html Expression_istask_ref.html
if test $? -ne 0 ; then
  exit 1
fi

cmp Expression_ismilestone.html Expression_ismilestone_ref.html
if test $? -ne 0 ; then
  exit 1
fi

/bin/rm Expression_*.html

exit 0

