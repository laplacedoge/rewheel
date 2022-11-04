#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utf8_codec.h"

static const char encoded_data_only_ascii[] = " _123abcABC@#$\t\r\n\a";

static const uint16_t decoded_data_only_ascii[] =
{
    0x0020, 0x005F, 0x0031, 0x0032, 0x0033, 0x0061, 0x0062, 0x0063,
    0x0041, 0x0042, 0x0043, 0x0040, 0x0023, 0x0024, 0x0009, 0x000D,
    0x000A, 0x0007
};

static const char encoded_data_only_greek_alphabet[] =
    "ΑαΒβΓγΔδΕεΖζΗηΘθΙιΚκΛλΜμΝνΞξΟοΠπΡρΣσςΤτΥυΦφΧχΨψΩω";

static const uint16_t decoded_data_only_greek_alphabet[] =
{
    0x0391, 0x03B1, 0x0392, 0x03B2, 0x0393, 0x03B3, 0x0394, 0x03B4,
    0x0395, 0x03B5, 0x0396, 0x03B6, 0x0397, 0x03B7, 0x0398, 0x03B8,
    0x0399, 0x03B9, 0x039A, 0x03BA, 0x039B, 0x03BB, 0x039C, 0x03BC,
    0x039D, 0x03BD, 0x039E, 0x03BE, 0x039F, 0x03BF, 0x03A0, 0x03C0,
    0x03A1, 0x03C1, 0x03A3, 0x03C3, 0x03C2, 0x03A4, 0x03C4, 0x03A5,
    0x03C5, 0x03A6, 0x03C6, 0x03A7, 0x03C7, 0x03A8, 0x03C8, 0x03A9,
    0x03C9
};

static const char encoded_data_only_chinese[] = "工欲善其事，必先利其器。";

static const uint16_t decoded_data_only_chinese[] =
{
    0x5DE5, 0x6B32, 0x5584, 0x5176, 0x4E8B, 0xFF0C, 0x5FC5, 0x5148,
    0x5229, 0x5176, 0x5668, 0x3002
};

static const char encoded_data_triple_mix[] =
    "\t<p>是的，first let's 设置 β to 1.9, 然后静观其变！</p>";

static const uint16_t decoded_data_triple_mix[] =
{
    0x0009, 0x003C, 0x0070, 0x003E, 0x662F, 0x7684, 0xFF0C, 0x0066,
    0x0069, 0x0072, 0x0073, 0x0074, 0x0020, 0x006C, 0x0065, 0x0074,
    0x0027, 0x0073, 0x0020, 0x8BBE, 0x7F6E, 0x0020, 0x03B2, 0x0020,
    0x0074, 0x006F, 0x0020, 0x0031, 0x002E, 0x0039, 0x002C, 0x0020,
    0x7136, 0x540E, 0x9759, 0x89C2, 0x5176, 0x53D8, 0xFF01, 0x003C,
    0x002F, 0x0070, 0x003E
};

void test_decode_right_encoded_data(void)
{
    uint16_t code_points[1024];
    size_t decoded_num;
    int ret;

    printf("\n>>> TEST: decode right encoded data\n");

    ret = utf8_decode(code_points, encoded_data_only_ascii,
                      sizeof(encoded_data_only_ascii) - 1,
                      &decoded_num, NULL, 0);
    assert(ret == UTF8DEC_OK);
    assert(decoded_num == 18);
    assert(memcmp(code_points, decoded_data_only_ascii, 18) == 0);

    ret = utf8_decode(code_points, encoded_data_only_greek_alphabet,
                      sizeof(encoded_data_only_greek_alphabet) - 1,
                      &decoded_num, NULL, 0);
    assert(ret == UTF8DEC_OK);
    assert(decoded_num == 49);
    assert(memcmp(code_points, decoded_data_only_greek_alphabet, 49) == 0);

    ret = utf8_decode(code_points, encoded_data_only_chinese,
                      sizeof(encoded_data_only_chinese) - 1,
                      &decoded_num, NULL, 0);
    assert(ret == UTF8DEC_OK);
    assert(decoded_num == 12);
    assert(memcmp(code_points, decoded_data_only_chinese, 12) == 0);

    ret = utf8_decode(code_points, encoded_data_triple_mix,
                      sizeof(encoded_data_triple_mix) - 1,
                      &decoded_num, NULL, 0);
    assert(ret == UTF8DEC_OK);
    assert(decoded_num == 43);
    assert(memcmp(code_points, decoded_data_triple_mix, 43) == 0);

    printf("<<< PASS\n");
}

static const char encoded_data_partial_invalid_triple_mix[] =
    "\t<p>是\x80的，first let\xFF'\xC3s 设置 β to 1.9, 然后\x80\xA7静观其变！</p>";

static const uint16_t decoded_data_partial_invalid_triple_mix[] =
{
    0x0009, 0x003C, 0x0070, 0x003E, 0x662F, 0x7684, 0xFF0C, 0x0066,
    0x0069, 0x0072, 0x0073, 0x0074, 0x0020, 0x006C, 0x0065, 0x0074,
    0x0027, 0x0073, 0x0020, 0x8BBE, 0x7F6E, 0x0020, 0x03B2, 0x0020,
    0x0074, 0x006F, 0x0020, 0x0031, 0x002E, 0x0039, 0x002C, 0x0020,
    0x7136, 0x540E, 0x9759, 0x89C2, 0x5176, 0x53D8, 0xFF01, 0x003C,
    0x002F, 0x0070, 0x003E
};

void test_decode_partial_invalid_encoded_data(void)
{
    uint16_t code_points[1024];
    size_t decoded_num;
    size_t last_pos;
    int ret;

    printf("\n>>> TEST: decode partial invalid encoded data\n");

    ret = utf8_decode(code_points,
                      encoded_data_partial_invalid_triple_mix,
                      sizeof(encoded_data_partial_invalid_triple_mix) - 1,
                      &decoded_num, &last_pos, 0);
    assert(ret == UTF8DEC_ERR_INVALID_DATA);
    assert(last_pos == 7);

    ret = utf8_decode(code_points,
                      encoded_data_partial_invalid_triple_mix,
                      sizeof(encoded_data_partial_invalid_triple_mix) - 1,
                      &decoded_num, NULL, UTF8DEC_IGNORE_INVALID_DATA);
    assert(ret == UTF8DEC_OK);
    assert(decoded_num == 43);
    assert(memcmp(code_points, decoded_data_partial_invalid_triple_mix, 43) == 0);

    printf("<<< PASS\n");
}

void test_flag_conflict(void)
{
    uint16_t code_points[1024];
    size_t decoded_num;
    size_t last_pos;
    int ret;

    printf("\n>>> TEST: flag conflict\n");

    ret = utf8_decode(code_points,
                      encoded_data_partial_invalid_triple_mix,
                      sizeof(encoded_data_partial_invalid_triple_mix) - 1,
                      &decoded_num, NULL, UTF8DEC_IGNORE_INVALID_DATA |
                                          UTF8DEC_REPLACE_UNKNOWN_CHAR);
    assert(ret == UTF8DEC_ERR_FLAG_CONFLICT);

    printf("<<< PASS\n");
}

static const uint16_t decoded_data_partial_invalid_triple_mix_with_unknown_char_replacement[] =
{
    0x0009, 0x003C, 0x0070, 0x003E, 0x662F, 0xFFFD, 0x7684, 0xFF0C,
    0x0066, 0x0069, 0x0072, 0x0073, 0x0074, 0x0020, 0x006C, 0x0065,
    0x0074, 0xFFFD, 0x0027, 0xFFFD, 0x0073, 0x0020, 0x8BBE, 0x7F6E,
    0x0020, 0x03B2, 0x0020, 0x0074, 0x006F, 0x0020, 0x0031, 0x002E,
    0x0039, 0x002C, 0x0020, 0x7136, 0x540E, 0xFFFD, 0xFFFD, 0x9759,
    0x89C2, 0x5176, 0x53D8, 0xFF01, 0x003C, 0x002F, 0x0070, 0x003E
};

void test_replace_unknown_char(void)
{
    uint16_t code_points[1024];
    size_t decoded_num;
    size_t last_pos;
    int ret;

    printf("\n>>> TEST: replace unknown character\n");

    ret = utf8_decode(code_points,
                      encoded_data_partial_invalid_triple_mix,
                      sizeof(encoded_data_partial_invalid_triple_mix) - 1,
                      &decoded_num, NULL, UTF8DEC_REPLACE_UNKNOWN_CHAR);
    assert(ret == UTF8DEC_OK);
    assert(decoded_num == 48);
    assert(memcmp(code_points, decoded_data_partial_invalid_triple_mix_with_unknown_char_replacement, 48) == 0);

    printf("<<< PASS\n");
}

int main(int argc, char **argv)
{
    test_decode_right_encoded_data();

    test_decode_partial_invalid_encoded_data();

    test_flag_conflict();

    test_replace_unknown_char();

    return 0;
}
