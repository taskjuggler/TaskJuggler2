#! /bin/sh

lines=`fgrep -c FooAccount Report_account.html`
if test $lines -ne 4 ; then
  echo "Report_account.html broken"
  exit 1
fi

lines=`fgrep -c FooResource Report_resource.html`
if test $lines -ne 4 ; then
  exit 1
fi

lines=`fgrep -c FooTask Report_task.html`
if test $lines -ne 4 ; then
  exit 1
fi

/bin/rm Report*.html

exit 0

