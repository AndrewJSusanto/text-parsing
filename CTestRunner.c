#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h> 
#include <fcntl.h>
#include <stdint.h>

/* Include correct data structure header */
#include "Dictionary.h"

#define FIRST_TEST Empty_length

static uint8_t disable_exit_handler;
jmp_buf test_crash;
int saved_stdout;
int devNull;

/* Define tests */
enum Test_e {
  Empty_size = 0,
  Free_test_1,
  Insert_size_1,
  Delete_size_1,
  Insert_find_1,
  Delete_find_1,
  Empty_equals,
  Insert_equals_1,
  Delete_equals_1,
  NUM_TESTS,
};

bool any_char(char ch, char *charArr) {
    while (*charArr != 0) {
        if (ch == *charArr++) {
            return true;
        }
    }
    return false;
}


/* Define test names */
char *testName(int test) {
  if (test == Empty_size) return "Empty_size";
  if (test == Free_test_1) return "Free_test_1";
  if (test == Insert_size_1) return "Insert_size_1";
  if (test == Delete_size_1) return "Delete_size_1";
  if (test == Delete_find_1) return "Delete_find_1";
  if (test == Insert_find_1) return "Insert_find_1";
  if (test == Empty_equals) return "Empty_equals";
  if (test == Insert_equals_1) return "Insert_equals_1";
  if (test == Delete_equals_1) return "Delete_equals_1";
  return "";
}

void dataPrinter_int(void*kv_void) {
  KVPair *kv = (KVPair *)kv_void;
  printf("%s:%ld ", kv->key, (intptr_t)kv->value);
}

void dataPrinter_str(void*kv_void) {
  KVPair *kv = (KVPair *)kv_void;
  printf("%s:%s ", kv->key, (char*)kv->value);
}

void freeKV_dynamic_val(KVPair *kv) {
  free(kv->key);
  free(kv->value);
  free(kv);
}

void freeKV_static_val(KVPair *kv) {
  free(kv->key);
  free(kv);
}

void freeKV_all_static(KVPair *kv) {
  free(kv);
}

void dummy_freeKV(KVPair *kv) {}

void test_free_kv(KVPair *kv) {
  kv->value = 0;
}

char *standardize_output(char *inString) {
  char *outString = calloc(sizeof(char), strlen(inString));
  size_t outOffset = 0;
  char c;
  char prevChar = ' '; //Ignore all leading space
  while ((c = *inString++) != 0) {
    if (any_char(c, " \t\n")) {
      if (prevChar != ' ') {
        outString[outOffset++] = ' ';
        prevChar = c;
      }
    } else {
      outString[outOffset++] = c;
      prevChar = c;
    }
  }
  
  if (outOffset > 0 && prevChar == ' ') {
   
    outString[outOffset - 1] = 0; // Cut off last space
  } else {
    outString[outOffset] = 0; // Add end of string
  }
  return outString;
}

char *get_output(Dictionary *d) {
  int out_pipe[2];
  
  //Setup stdout redirection
  pipe(out_pipe);
  dup2(out_pipe[1], STDOUT_FILENO);
  //Call function
  dictionary_print(d);
  //Close stdout redirection
  fflush(stdout);
  dup2(saved_stdout, STDOUT_FILENO);
  close(out_pipe[1]);
  /* read from pipe into buffer */
  char* string = calloc(100, sizeof(char));
  size_t buf_size = 100;
  int offset = 0;
  int read_bytes = 0;
  while ((read_bytes = read(out_pipe[0], string + offset, 100)) == 100) {
    char *newBuf = realloc(string, buf_size + 100);
    buf_size += 100;
    if (newBuf == NULL) {
      exit(88);
    }
    free(string);
    string = newBuf;
    offset += read_bytes;
  } 
  close(out_pipe[0]);
  return string;
}

KVPair *create_kv_str(char *k, char *val) {
  KVPair *kv = (KVPair*)malloc(sizeof(KVPair));
  kv->key = k;
  kv->value = val;
  return kv;
}


uint8_t runTest(int test) {

  switch(test) {
    case Empty_size: {
      Dictionary *d = dictionary_create(5, dataPrinter_int, freeKV_static_val);
      if (dictionary_size(d) != 0) return 1;
        return 0;
    }
    case Free_test_1: {
      Dictionary *d = dictionary_create(5, dataPrinter_int, dummy_freeKV);
      dictionary_destroy(d, false);
      return 0;
    }
    case Insert_size_1: {
      
      Dictionary *d = dictionary_create(2, dataPrinter_int, dummy_freeKV);
      KVPair kv;
      kv.key = "Some value";
      kv.value = (void *)5;
      if (!dictionary_insert(d, &kv)) return 1;
      if (dictionary_size(d) != 1) return 2;
      return 0;
    }
    case Insert_find_1: {
      Dictionary *d = dictionary_create(2, dataPrinter_int, dummy_freeKV);
      KVPair kv;
      kv.key = "Some value";
      kv.value = (void *)5;
      if (!dictionary_insert(d, &kv)) return 1;
      if (dictionary_find(d, "Some value") != &kv) return 2;
      return 0;
    }
    case Delete_size_1: {
      Dictionary *d = dictionary_create(2, dataPrinter_int, dummy_freeKV);
      KVPair kv;
      kv.key = "Some value";
      kv.value = (void *)5;
      KVPair kv3;
      kv3.key = "key3";
      kv3.value = (void *)32;
      KVPair kv4;
      kv4.key = "key4";
      kv4.value = (void *)12;

      char *kvKey = calloc(sizeof(char), strlen(kv.key));
      strcpy(kvKey, kv.key);
      dictionary_insert(d, &kv);
      dictionary_insert(d, &kv3);
      dictionary_insert(d, &kv4);
      
      //Test key not in dict
      if (dictionary_delete(d, "key5") != NULL) return 1;
      //Test key in dict
      if (dictionary_delete(d, kv3.key) != &kv3) return 2;
      if (dictionary_size(d) != 2) return 3;
      return 0;
    }
    case Delete_find_1: {
      Dictionary *d = dictionary_create(2, dataPrinter_int, dummy_freeKV);
      //Ensure works for simple insert
      KVPair kv;
      kv.key = "Some value";
      kv.value = (void *)5;
      KVPair kv3;
      kv3.key = "key3";
      kv3.value = (void *)32;
      KVPair kv4;
      kv4.key = "key4";
      kv4.value = (void *)12;

      char *kvKey = calloc(sizeof(char), strlen(kv.key));
      strcpy(kvKey, kv.key);
      dictionary_insert(d, &kv);
      dictionary_insert(d, &kv3);
      dictionary_insert(d, &kv4);

      //Test exact string
      if (dictionary_find(d, kv3.key) != &kv3) return 1;
      if (dictionary_delete(d, kv3.key) != &kv3) return 2;
      if (dictionary_find(d, kv3.key) != NULL) return 3;

      return 0;
    }
    
    case Empty_equals: {
        Dictionary *d = dictionary_create(2, dataPrinter_str, dummy_freeKV);
        char *s = get_output(d);
        char *f_s = standardize_output(s);
        
        if (*f_s != '\000') {
            printf("[Expect]: \"\n\"\n");
            printf("[Actual]: \"%s\"\n", s);
            free(f_s);
            return 1;
        }
        free(s);
        free(f_s);
        return 0;
    }
    case Insert_equals_1: {
        Dictionary *d = dictionary_create(10, dataPrinter_str, dummy_freeKV);
        dictionary_insert(d, create_kv_str("k1", "v1"));
        dictionary_insert(d, create_kv_str("k2", "v2"));
        dictionary_insert(d, create_kv_str("k3", "v3"));
        dictionary_insert(d, create_kv_str("k4", "v4"));
        dictionary_insert(d, create_kv_str("k5", "v5"));
        char *string = get_output(d);
        char *f_s = standardize_output(string);
        if (strcmp(f_s, "k2:v2 k3:v3 k4:v4 k5:v5 k1:v1") != 0) {
            printf("[Expect]: \"k2:v2 k3:v3 k4:v4 k5:v5 k1:v1\n\"\n");
            printf("[Actual]: \"%s\"\n", string);
            free(string);
            free(f_s);
            return 1;
        }
        free(string);
        free(f_s);
        
        return 0;
    }
    case Delete_equals_1: {
        Dictionary *d = dictionary_create(10, dataPrinter_str, dummy_freeKV);
        dictionary_insert(d, create_kv_str("k1", "v1"));
        dictionary_insert(d, create_kv_str("k2", "v2"));
        dictionary_insert(d, create_kv_str("k3", "v3"));
        dictionary_insert(d, create_kv_str("k4", "v4"));
        dictionary_insert(d, create_kv_str("k5", "v5"));
        dictionary_delete(d, "k3");
        char *string = get_output(d);
        char *f_s = standardize_output(string);
        if (strcmp(f_s, "k2:v2 k4:v4 k5:v5 k1:v1") != 0) {
            printf("[Expect]: \"k2:v2 k4:v4 k5:v5 k1:v1\n\"\n");
            printf("[Actual]: \"%s\"\n", string);
            free(string);
            free(f_s);
            return 1;
        }
        free(string);
        free(f_s);
        
        return 0;
    }
  }
  return 255;
}

void exit_attempt_handler(void) {
  if (disable_exit_handler) return;
  exit(30);
}


int main (int argc, char **argv) {

  if (argc > 2 || argc  <= 1) {
    printf("Usage: %s [<index_of_test>|-tests]\n", argv[0]);
    exit(51);
  }
  if (strcmp(argv[1], "-tests") == 0) {
    for (int i = 0; i < NUM_TESTS; i++) {
      printf("%d %s\n", i, testName(i));
    }
  } else {
    int test_index = atoi(argv[1]);
    if (test_index < 0 || test_index >= NUM_TESTS) {
      printf("Test index is out of bounds");
      exit(50);
    }

    saved_stdout = dup(1);
    devNull = open("/dev/null", O_WRONLY);
    atexit(exit_attempt_handler);
    
    disable_exit_handler = 0;
    int test_output = runTest(test_index);
    disable_exit_handler = 1;
    exit(test_output);
  }
  
  
}