BEGIN {
    print "#include <ramfs.h>"
}

{
    print "extern char _binary_" $1 "_end[];"
    print "extern char _binary_" $1 "_start[];"
}