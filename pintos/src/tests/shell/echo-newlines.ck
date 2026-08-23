# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
line1
line2 carriage
return 
echo: exit(0)
EOF
pass;