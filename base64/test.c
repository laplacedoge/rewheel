#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "base64.h"

char buff[1024];

static const char base64_raw_0_data[] = "";
static const char base64_enc_0_data[] = "";

static const char base64_raw_0_padding[] =
    "Ai is a beautiful, young girl with blonde hair that is usually tied up at"
    " the left side of her head with a blue scrunchie and blue-eyes";

static const char base64_enc_0_padding[] =
    "QWkgaXMgYSBiZWF1dGlmdWwsIHlvdW5nIGdpcmwgd2l0aCBibG9uZGUgaGFpciB0aGF0IGlzIH"
    "VzdWFsbHkgdGllZCB1cCBhdCB0aGUgbGVmdCBzaWRlIG9mIGhlciBoZWFkIHdpdGggYSBibHVl"
    "IHNjcnVuY2hpZSBhbmQgYmx1ZS1leWVz";

static const char base64_raw_1_padding[] =
    "In reality, she is just a kind girl"
    " who deeply loves her mistress";

static const char base64_enc_1_padding[] =
    "SW4gcmVhbGl0eSwgc2hlIGlzIGp1c3QgYSBraW5kIGdpcmwgd2hvIGRlZXBseSBsb3ZlcyBoZX"
    "IgbWlzdHJlc3M=";

static const char base64_raw_2_padding[] =
    "Ai Hayasaka is one of the protagonists in"
    " the Kaguya-sama wa Kokurasetai series";

static const char base64_enc_2_padding[] =
    "QWkgSGF5YXNha2EgaXMgb25lIG9mIHRoZSBwcm90YWdvbmlzdHMgaW4gdGhlIEthZ3V5YS1zYW"
    "1hIHdhIEtva3VyYXNldGFpIHNlcmllcw==";

void test_encoding(void)
{
    int len;

    printf("\n>>> TEST: try to encode some data\n");

    len = base64_encode(&buff, &base64_raw_0_data,
                        strlen(base64_raw_0_data));
    assert(len == strlen(base64_enc_0_data));

    len = base64_encode(&buff, &base64_raw_0_padding,
                        strlen(base64_raw_0_padding));
    assert(len == strlen(base64_enc_0_padding));
    assert(memcmp(&buff, &base64_enc_0_padding, len) == 0);

    len = base64_encode(&buff, &base64_raw_1_padding,
                        strlen(base64_raw_1_padding));
    assert(len == strlen(base64_enc_1_padding));
    assert(memcmp(&buff, &base64_enc_1_padding, len) == 0);

    len = base64_encode(&buff, &base64_raw_2_padding,
                        strlen(base64_raw_2_padding));
    assert(len == strlen(base64_enc_2_padding));
    assert(memcmp(&buff, &base64_enc_2_padding, len) == 0);

    printf("<<< PASS\n");
}

static const char base64_enc_invalid_len[] =
    "SW4gYm90aCBjaGFwdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlci"
    "BicmVhaw";

static const char base64_enc_invalid_char_in_data[] =
    "SW4gYm90aCBjaG*wdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlci"
    "BicmVhaw==";

static const char base64_enc_invalid_char_in_padding[] =
    "SW4gYm90aCBjaGFwdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlci"
    "BicmVhaw=&";

static const char base64_enc_too_many_padding_char[] =
    "SW4gYm90aCBjaGFwdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlci"
    "BicmVhaw======";

static const char base64_enc_invalid_padding_char_position[] =
    "SW4gYm90aCBjaGFwdGV==ycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRl"
    "ciBicmVhaw====";

void test_decoding(void)
{
    int ret;

    printf("\n>>> TEST: try to decode some data\n");

    ret = base64_decode(&buff, &base64_enc_0_data,
                        strlen(base64_enc_0_data));
    assert(ret == strlen(base64_raw_0_data));

    ret = base64_decode(&buff, &base64_enc_0_padding,
                        strlen(base64_enc_0_padding));
    assert(ret == strlen(base64_raw_0_padding));
    assert(memcmp(&buff, base64_raw_0_padding, ret) == 0);

    ret = base64_decode(&buff, &base64_enc_1_padding,
                        strlen(base64_enc_1_padding));
    assert(ret == strlen(base64_raw_1_padding));
    assert(memcmp(&buff, base64_raw_1_padding, ret) == 0);

    ret = base64_decode(&buff, &base64_enc_2_padding,
                        strlen(base64_enc_2_padding));
    assert(ret == strlen(base64_raw_2_padding));
    assert(memcmp(&buff, base64_raw_2_padding, ret) == 0);

    ret = base64_decode(&buff, &base64_enc_invalid_len,
                        strlen(base64_enc_invalid_len));
    assert(ret == BASE64_ERR_INVALID_ENC_LEN);

    ret = base64_decode(&buff, &base64_enc_invalid_char_in_data,
                        strlen(base64_enc_invalid_char_in_data));
    assert(ret == BASE64_ERR_INVALID_ENC_CHAR);
    assert(base64_error_param() == 14);

    ret = base64_decode(&buff, &base64_enc_invalid_char_in_padding,
                        strlen(base64_enc_invalid_char_in_padding));
    assert(ret == BASE64_ERR_INVALID_ENC_CHAR);
    assert(base64_error_param() == 83);

    ret = base64_decode(&buff, &base64_enc_too_many_padding_char,
                        strlen(base64_enc_too_many_padding_char));
    assert(ret == BASE64_ERR_INVALID_ENC_PADDING);
    assert(base64_error_param() == 82);

    ret = base64_decode(&buff, &base64_enc_invalid_padding_char_position,
                        strlen(base64_enc_invalid_padding_char_position));
    assert(ret == BASE64_ERR_INVALID_ENC_PADDING);
    assert(base64_error_param() == 19);

    printf("<<< PASS\n");
}

int main(int argc, char *argv[])
{
    test_encoding();

    test_decoding();

    return 0;
}
