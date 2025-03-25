

void syscall_exit(int error_code) {
    terminal_writestring("exiting!");
    char myString[2] = { (char)error_code + '0', '\0' };
    terminal_writestring(myString);
    return 0;
}