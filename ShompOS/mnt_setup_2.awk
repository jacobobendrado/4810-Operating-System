BEGIN {
    print "int init_mnt(ramfs_dir_t *mnt) {"
}

{
    print "ramfs_create_file(mnt, \"" $2 "\", _binary_" $1 "_start, _binary_" $1 "_end - _binary_" $1 "_start);"
}

END {
    print "return 0;"
    print "}"
}