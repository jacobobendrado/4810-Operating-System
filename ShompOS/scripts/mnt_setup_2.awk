# mnt_setup_2.awk
# Function definition for init_mnt in mnt.c
# Cedarville University 2024-25 OSDev Team

# Print function signature with the __mnt directory
/^_/ {
    print "int init_mnt(ramfs_dir_t *" $1 ") {"
}

# Create file in ShompOS for each file
# TODO: Handle possible failure to create file
/^f/ {
    print "ramfs_create_file(" $2 ", \"" $3 "\", _binary_" $4 "_start, _binary_" $4 "_end - _binary_" $4 "_start);"
}

# Create directory in ShompOS for each file
# TODO: Handle possible failure to create directory
/^d/ {
    print "ramfs_dir_t *" $4 " = ramfs_create_dir(" $2 ", \"" $3 "\");"
}

# Finish function
END {
    print "return 0;"
    print "}"
}