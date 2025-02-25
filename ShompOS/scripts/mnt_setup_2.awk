/^_/ {
    print "int init_mnt(ramfs_dir_t *" $1 ") {"
}

/^f/ {
    print "ramfs_create_file(" $2 ", \"" $3 "\", _binary_" $4 "_start, _binary_" $4 "_end - _binary_" $4 "_start);"
}

/^d/ {
    print "ramfs_dir_t *" $4 " = ramfs_create_dir(" $2 ", \"" $3 "\");"
}

END {
    print "return 0;"
    print "}"
}