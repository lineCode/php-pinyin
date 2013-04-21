/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

extern "C" {
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include "php_pinyin.h"

ZEND_DECLARE_MODULE_GLOBALS(pinyin)

zend_class_entry *pinyin_ce;
enum Dict_Type;


/* {{{ pinyin_methods[]
 *
 * Every user visible method must have an entry in pinyin_methods[].
 */

static zend_function_entry pinyin_methods[] = {
	PHP_ME(Pinyin, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Pinyin, __destruct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(Pinyin, loadDict, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Pinyin, convert, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Pinyin, multiConvert, NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Pinyin, exact, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ pinyin_module_entry
 */
zend_module_entry pinyin_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"pinyin",
	pinyin_methods,
	PHP_MINIT(pinyin),
	PHP_MSHUTDOWN(pinyin),
	PHP_RINIT(pinyin),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(pinyin),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(pinyin),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_PINYIN_VERSION, /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_PINYIN
extern "C" {
ZEND_GET_MODULE(pinyin)
}
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("pinyin.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_pinyin_globals, pinyin_globals)
    STD_PHP_INI_ENTRY("pinyin.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_pinyin_globals, pinyin_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_pinyin_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_pinyin_init_globals(zend_pinyin_globals *pinyin_globals)
{
	pinyin_globals->global_value = 0;
	pinyin_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pinyin)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
    zend_class_entry ce;
    INIT_CLASS_ENTRY(ce, "Pinyin", pinyin_methods);
    pinyin_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_class_constant_long(pinyin_ce, "TONE_DICT", strlen("TONE_DICT"), TONE_DICT);
    zend_declare_class_constant_long(pinyin_ce, "TONE_DYZ_DICT", strlen("TONE_DYZ_DICT"), TONE_DYZ_DICT);
    zend_declare_class_constant_long(pinyin_ce, "TONE_DUOYONG_DICT", strlen("TONE_DUOYONG_DICT"), TONE_DUOYONG_DICT);
    zend_declare_class_constant_long(pinyin_ce, "BME_DICT", strlen("BME_DICT"), BME_DICT);
    
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pinyin)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pinyin)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pinyin)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pinyin)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "pinyin support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

PHP_METHOD(Pinyin, __construct)
{
    PINYIN_G(pynotation) = IPYFactory::getPYInstance();
}

PHP_METHOD(Pinyin, __destruct)
{
    delete PINYIN_G(pynotation);
    PINYIN_G(pynotation) = NULL;
}

PHP_METHOD(Pinyin, loadDict)
{
	char *path = NULL;
	int len;
    enum Dict_Type dict_type;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &path, &len, &dict_type) == FAILURE) {
		return;
	}

    switch(dict_type) {
        case TONE_DICT:
            if(!PINYIN_G(pynotation)->loadDict(path)) RETURN_FALSE;
            break;
        case TONE_DYZ_DICT:
            if(!PINYIN_G(pynotation)->loadDyzDict(path)) RETURN_FALSE;
            break;
        case TONE_DUOYONG_DICT:
            if(!PINYIN_G(pynotation)->loadDYDict(path)) RETURN_FALSE;
            break;
        case BME_DICT:
            if(!PINYIN_G(pynotation)->loadBMEDict(path)) RETURN_FALSE;
            break;
        default:
            RETURN_FALSE;
            break;
    }
    RETURN_TRUE;
}

PHP_METHOD(Pinyin, convert)
{
	char *characters = NULL;
	int  len;
    zend_bool get_tone = 0;
    bool result = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &characters, &len, &get_tone) == FAILURE) {
		RETURN_NULL();
	}
    vector<string> py_result;
    if(get_tone) {
        result = PINYIN_G(pynotation)->convertToTonePY(characters, &py_result);
    } else {
        result = PINYIN_G(pynotation)->convertToPY(characters, &py_result); 
    }
    if(result) {
        array_init(return_value);
        vector<string>::iterator _it = py_result.begin();
        int i = 0;
        for(vector<string>::iterator _sit = py_result.begin(); _sit != py_result.end(); _sit++) {
            add_index_string(return_value, i++, (*_sit).c_str(), 1);
        }
    } else {
        RETURN_FALSE;
    }
}

PHP_METHOD(Pinyin, multiConvert)
{

}

PHP_METHOD(Pinyin, exact)
{

}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
