# mnt_setup_1.awk
# Initial text in mnt.c file
# Cedarville University 2024-25 OSDev Team

# Initial includes
BEGIN {
    print "// AUTO-GENERATED FILE. DO NOT EDIT."
    print "#include <ramfs.h>"
}

# Declaring variables for each file
/^f/ {
    print "extern char _binary_" $4 "_end[];"
    print "extern char _binary_" $4 "_start[];"
}