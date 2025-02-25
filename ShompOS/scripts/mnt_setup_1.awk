BEGIN {
    print "#include <ramfs.h>"
}

/^f/ {
    print "extern char _binary_" $4 "_end[];"
    print "extern char _binary_" $4 "_start[];"
}