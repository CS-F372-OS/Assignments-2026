# -*- perl -*-
use strict;
use warnings;
use tests::tests;

check_expected([<<'EOF']);
testfile1.txt: created
testfile2.txt: created
touch: exit(0)
EOF
pass;