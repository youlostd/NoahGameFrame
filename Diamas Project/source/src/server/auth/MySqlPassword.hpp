#ifndef METIN2_SERVER_AUTH_MYSQLPASSWORD_HPP
#define METIN2_SERVER_AUTH_MYSQLPASSWORD_HPP

/*
    MySQL 4.1.1 password hashing: SHA conversion (see RFC 2289, 3174) twice
    applied to the password string, and then produced octet sequence is
    converted to hex string.
    The result of this function is used as return value from PASSWORD() and
    is stored in the database.
  SYNOPSIS
    my_make_scrambled_password_sha1()
    buf       OUT buffer of size 2*SHA1_HASH_SIZE + 2 to store hex string
    password  IN  password string
    pass_len  IN  length of password string
*/

void my_make_scrambled_password_sha1(char *to, const char *password,
                                     size_t pass_len);

#endif
