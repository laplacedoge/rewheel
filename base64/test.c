#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "base64.h"

char buff[1024];

static const char raw_0_data[] = "";
static const char enc_0_data[] = "";

static const char raw_0_padding[] = "Ai is a beautiful, young girl with blonde hair that is usually tied up at the left side of her head with a blue scrunchie and blue-eyes";
static const char enc_0_padding[] = "QWkgaXMgYSBiZWF1dGlmdWwsIHlvdW5nIGdpcmwgd2l0aCBibG9uZGUgaGFpciB0aGF0IGlzIHVzdWFsbHkgdGllZCB1cCBhdCB0aGUgbGVmdCBzaWRlIG9mIGhlciBoZWFkIHdpdGggYSBibHVlIHNjcnVuY2hpZSBhbmQgYmx1ZS1leWVz";

static const char raw_1_padding[] = "In reality, she is just a kind girl who deeply loves her mistress";
static const char enc_1_padding[] = "SW4gcmVhbGl0eSwgc2hlIGlzIGp1c3QgYSBraW5kIGdpcmwgd2hvIGRlZXBseSBsb3ZlcyBoZXIgbWlzdHJlc3M=";

static const char raw_2_padding[] = "Ai Hayasaka is one of the protagonists in the Kaguya-sama wa Kokurasetai series";
static const char enc_2_padding[] = "QWkgSGF5YXNha2EgaXMgb25lIG9mIHRoZSBwcm90YWdvbmlzdHMgaW4gdGhlIEthZ3V5YS1zYW1hIHdhIEtva3VyYXNldGFpIHNlcmllcw==";

void test_encoding(void)
{
    int len;

    assert((len = base64_encode(&buff, &raw_0_data, strlen(raw_0_data))) == strlen(enc_0_data));

    assert((len = base64_encode(&buff, &raw_0_padding, strlen(raw_0_padding))) == strlen(enc_0_padding));
    assert(memcmp(&buff, &enc_0_padding, len) == 0);

    assert((len = base64_encode(&buff, &raw_1_padding, strlen(raw_1_padding))) == strlen(enc_1_padding));
    assert(memcmp(&buff, &enc_1_padding, len) == 0);

    assert((len = base64_encode(&buff, &raw_2_padding, strlen(raw_2_padding))) == strlen(enc_2_padding));
    assert(memcmp(&buff, &enc_2_padding, len) == 0);
}

static const char enc_invalid_len[] = "SW4gYm90aCBjaGFwdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlciBicmVhaw";
static const char enc_invalid_char_in_data[] = "SW4gYm90aCBjaG*wdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlciBicmVhaw==";
static const char enc_invalid_char_in_padding[] = "SW4gYm90aCBjaGFwdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlciBicmVhaw=&";
static const char enc_too_many_padding_char[] = "SW4gYm90aCBjaGFwdGVycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlciBicmVhaw======";
static const char enc_invalid_padding_char_position[] = "SW4gYm90aCBjaGFwdGV==ycyAxNTIgYW5kIDE2Miwgd2Ugc2VlIHRoYXQgZHVyaW5nIHdpbnRlciBicmVhaw====";

void test_decoding(void)
{
    int ret;

    ret = base64_decode(&buff, &enc_0_data, strlen(enc_0_data));
    assert(ret == strlen(raw_0_data));

    ret = base64_decode(&buff, &enc_0_padding, strlen(enc_0_padding));
    assert(ret == strlen(raw_0_padding));
    assert(memcmp(&buff, raw_0_padding, ret) == 0);

    ret = base64_decode(&buff, &enc_1_padding, strlen(enc_1_padding));
    assert(ret == strlen(raw_1_padding));
    assert(memcmp(&buff, raw_1_padding, ret) == 0);

    ret = base64_decode(&buff, &enc_2_padding, strlen(enc_2_padding));
    assert(ret == strlen(raw_2_padding));
    assert(memcmp(&buff, raw_2_padding, ret) == 0);

    ret = base64_decode(&buff, &enc_invalid_len, strlen(enc_invalid_len));
    assert(ret == BASE64_ERR_INVALID_ENC_LEN);

    ret = base64_decode(&buff, &enc_invalid_char_in_data, strlen(enc_invalid_char_in_data));
    assert(ret == BASE64_ERR_INVALID_ENC_CHAR);
    assert(base64_error_param() == 14);

    ret = base64_decode(&buff, &enc_invalid_char_in_padding, strlen(enc_invalid_char_in_padding));
    assert(ret == BASE64_ERR_INVALID_ENC_CHAR);
    assert(base64_error_param() == 83);

    ret = base64_decode(&buff, &enc_too_many_padding_char, strlen(enc_too_many_padding_char));
    assert(ret == BASE64_ERR_INVALID_ENC_PADDING);
    assert(base64_error_param() == 82);

    ret = base64_decode(&buff, &enc_invalid_padding_char_position, strlen(enc_invalid_padding_char_position));
    assert(ret == BASE64_ERR_INVALID_ENC_PADDING);
    assert(base64_error_param() == 19);
}

int main(int argc, char *argv[])
{
    test_encoding();
    test_decoding();

    printf("TEST PASS!");

    return 0;
}
