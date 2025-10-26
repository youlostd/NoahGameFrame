#include "MySqlPassword.hpp"
using uint32 = uint32_t;

#include <thecore/sha1.h>

/* Character to use as version identifier for version 4.1 */

#define PVERSION41_CHAR '*'

namespace
{

char _dig_vec_upper[] =
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/*
    Convert given octet sequence to asciiz string of hex characters;
    str..str+len and 'to' may not overlap.
  SYNOPSIS
    octet2hex()
    buf       OUT output buffer. Must be at least 2*len+1 bytes
    str, len  IN  the beginning and the length of the input string

  RETURN
    buf+len*2
*/
static char *octet2hex(char *to, const char *str, size_t len)
{
  const char *str_end= str + len;
  for (; str != str_end; ++str)
  {
    *to++= _dig_vec_upper[((uint8_t) *str) >> 4];
    *to++= _dig_vec_upper[((uint8_t) *str) & 0x0F];
  }
  *to= '\0';
  return to;
}


/**
  Wrapper function to compute SHA1 message digest.

  @param digest [out]  Computed SHA1 digest
  @param buf    [in]   Message to be computed
  @param len    [in]   Length of the message

  @return              void
*/
static void compute_sha1_hash(uint8_t *digest, const char *buf, size_t len)
{
  SHA1_CTX sha1_context;

  SHA1Init(&sha1_context);
  SHA1Update(&sha1_context, (const uint8_t *) buf, len);
  SHA1Final(digest, &sha1_context);
}


/**
  Compute two stage SHA1 hash of the password :

    hash_stage1=sha1("password")
    hash_stage2=sha1(hash_stage1)

  @param password    [IN]   Password string.
  @param pass_len    [IN]   Length of the password.
  @param hash_stage1 [OUT]  sha1(password)
  @param hash_stage2 [OUT]  sha1(hash_stage1)
*/
inline static
void compute_two_stage_sha1_hash(const char *password, size_t pass_len,
                                 uint8_t *hash_stage1, uint8_t *hash_stage2)
{
  /* Stage 1: hash password */
  compute_sha1_hash(hash_stage1, password, pass_len);

  /* Stage 2 : hash first stage's output. */
  compute_sha1_hash(hash_stage2, (const char *) hash_stage1, 20);
}

}

/*
    MySQL 4.1.1 password hashing: SHA conversion (see RFC 2289, 3174) twice
    applied to the password string, and then produced octet sequence is
    converted to hex string.
    The result of this function is used as return value from PASSWORD() and
    is stored in the database.
  SYNOPSIS
    my_make_scrambled_password_sha1()
    buf       OUT buffer of size 2*SHA1_DIGEST_LENGTH + 2 to store hex string
    password  IN  password string
    pass_len  IN  length of password string
*/
void my_make_scrambled_password_sha1(char *to, const char *password,
                                     size_t pass_len)
{
  uint8_t hash_stage2[20];

  /* Two stage SHA1 hash of the password. */
  compute_two_stage_sha1_hash(password, pass_len, (uint8_t *) to, hash_stage2);

  /* convert hash_stage2 to hex string */
  *to++= PVERSION41_CHAR;
  octet2hex(to, (const char*) hash_stage2, 20);
}
