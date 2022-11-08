# BASE64
Base64 codec implementation in C.  

## Usage

### Example
Encode binary data:  
```c
/* some binary data */
const char just_data[] =
{
    0xE5, 0xB7, 0xA5, 0xE6, 0xAC, 0xB2, 0xE5, 0x96,
    0x84, 0xE5, 0x85, 0xB6, 0xE4, 0xBA, 0x8B, 0xEF,
    0xBC, 0x8C, 0xE5, 0xBF, 0x85, 0xE5, 0x85, 0x88,
    0xE5, 0x88, 0xA9, 0xE5, 0x85, 0xB6, 0xE5, 0x99,
    0xA8, 0xE3, 0x80, 0x82
};

/* buffer used to store encoded data */
char buff[1024];

/* store the size of the encoded data */
int size;

/* encode our binary data and print it */
size = base64_encode(buff, just_data, sizeof(just_data));
printf("encoded data: %.*s\n", size, buff);

/* console output */
// encoded data: 5bel5qyy5ZaE5YW25LqL77yM5b+F5YWI5Yip5YW25Zmo44CC
```
Decode base64 encoded data:  
```c
/* base64 encoded data */
const char just_data[] = "5bel5qyy5ZaE5YW25LqL77yM5b+F5YWI5Yip5YW25Zmo44CC";

/* buffer used to store decoded data */
char buff[1024];

/* store the size of the decoded data */
int ret;

/* decode our base64 encoded data and print it */
ret = base64_decode(buff, just_data, strlen(just_data));
if (ret > 0)
{
    printf("decoded data: %.*s\n", ret, buff);
}
else
{
    printf("failed to decode data, ret: %d\n", ret);
}

/* console output */
// decoded data: 工欲善其事，必先利其器。
```

### Error types
All possible error types while encoding or decoding, they are defined in `base64_err_t` in base64.c.
- `BASE64_OK`  
  Currently only for base64 module internal use.
- `BASE64_ERR_BAD_ARG`  
  Indicate that there is some problem with the argument passed to the function.
- `BASE64_ERR_BAD_ENC_CHAR`  
  Encounter a invalid character while validating the encoded data. Also, you can use `base64_error_param()` to get the index of the invalid character in the encoded data after this error occurred.
- `BASE64_ERR_BAD_ENC_PADDING`  
  Encounter invalid padding while validating the encoded data. Also, you can use `base64_error_param()` to get the index of the first invalid padding character in the encoded data after this error occurred.

### Building
This module only consists of two single files, just add them to your project and it will work fine.  

### Testing
Run `make test` to make unit test of this module.  
