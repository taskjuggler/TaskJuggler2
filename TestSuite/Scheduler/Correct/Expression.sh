#! /bin/sh

for i in containstask issubtaskof istask ismilestone istaskstatus ; do 
  cmp Expression_${i}.html Expression_${i}_ref.html
  if test $? -ne 0 ; then
    exit 1
  fi
done

/bin/rm Expression_*.html

exit 0

