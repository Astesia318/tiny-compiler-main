#pragma D option quiet

pid$target::malloc:entry { @m=count(); }
pid$target::free:entry   { @f=count(); }

END
{
  printa("malloc=%@d  free=%@d  \n", @m, @f);
}