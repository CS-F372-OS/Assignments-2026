# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
arg1 arg2 arg3 arg4 arg5 
echo: exit(0)
EOF
pass;
