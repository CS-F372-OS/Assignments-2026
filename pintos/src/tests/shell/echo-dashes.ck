# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
--help -n --version 
echo: exit(0)
EOF
pass;
