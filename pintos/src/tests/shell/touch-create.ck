# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
newfile.txt: created
touch: exit(0)
EOF
pass;