// Implement the text process application
// The output function is provided to you.
#include "Dictionary.h"
#include "List.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

//This is used internally by the output function. You don't need to change this value
#define WORD_BUFFER_SIZE 50

int sortCompare(void *obj1, void *obj2) { // Comparison function for sorting, takes into account value, key (and alphabetical order)
    KVPair *temp1 = (KVPair*)obj1;
    KVPair *temp2 = (KVPair*)obj2;
    int comparison = strcmp(temp1->key, temp2->key) * -1;

    if(temp1->value == temp2->value) { // compares key only if values of both are equal
        return comparison; // if values are equal, judge sorting by the strcmp of temp1 vs temp2
    }
    else if(temp1->value > temp2->value) {
        return 1;
    }
    else { // if temp1 < temp2
        return -1;
    }
    // strcmp returns 0 if equal, 1 if temp1 > temp2, -1 if temp2 is greater.
}

void dataPrinter(void *data){ // Prints data
    KVPair *temp = (KVPair*)data;
    printf("%s:%d\n", (char*)temp->key, temp->value);
}

void freeData(KVPair *data) { // Frees data
    free(data->key);
    free(data->value);
    free(data);
}


/**
 * @brief Checks if a character is in the char array.
 * 
 * @param ch The character to check
 * @param charArr The ckaracter array to compare against
 * @return true ch is in charArr
 * @return false ch is not in charArr
 */
bool any_char(char ch, char *charArr) {
    while (*charArr != 0) {
        if (ch == *charArr++) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Gets the distance to the closest delimeter.
 * 
 * @param word The string to look through
 * @param delim The delimeter(s) to search for
 * @return uint8_t The number of characters in the word 
 */
uint8_t word_length(char *str, char *delims) {
    int i = 0;
    while (!any_char(str[i], delims) && str[i] != 0) {
        i++;
    }
    return i;
}

/**
 * @brief Prints the output to stdout. Accepts one line of output at a time.
 * 
 * @param wordFreqs A dictionary of words to format according to their frequency
 * @param wordLengths A dictionary of words to format according to their length
 * @param text One output line to format
 */

void printOutput(Dictionary *wordFreqs, Dictionary *wordLengths, char *text) {
    char wordBuffer[WORD_BUFFER_SIZE] = {0};

    while (*text != 0) {
        uint8_t length = word_length(text, " \n");
        assert(length < WORD_BUFFER_SIZE - 1);
        memcpy(wordBuffer, text, length);
        text = text + length;
        wordBuffer[length] = 0;
        KVPair *freq = dictionary_find(wordFreqs, wordBuffer);
        KVPair *len = dictionary_find(wordLengths, wordBuffer);
        if (freq == NULL && len == NULL) {
            printf("%s ", wordBuffer);
        } else {
            printf("<span style=\"");
            if (freq != NULL) {
                uint8_t r = (intptr_t)freq->value * 23;
                uint8_t g = (intptr_t)freq->value * 29;
                uint8_t b = (intptr_t)freq->value * 31;
                printf("color:#%02x%02x%02x;font-weight:bold;", r, g, b);
            }
            if (len != NULL) {
                printf("font-size:%ldpx;", 2 * (intptr_t)len->value);
            }
            printf("\">%s</span> ", wordBuffer);
        }

        if (*text != 0) { // If delimiter is not null byte, then skip over it. 
            text += 1;
        }
    }
    printf("<br/>\n");
}

int main(void) {
    int HASHTABLE_SIZE = 100;
    char stopwords[BUFSIZ] = "";
    char words[BUFSIZ] = "";
    char delim[BUFSIZ] = " \n";



    fgets(stopwords, sizeof(stopwords), stdin); // reading line of stopwords from infile
    Dictionary *stop_dict = dictionary_create(HASHTABLE_SIZE, (void*)dataPrinter, freeData);
    for(char* parse = strtok(stopwords, delim); parse; parse = strtok(NULL, delim)) { // tokenize line word for word delimited by whitespace and newline, then insert into dictionary
        KVPair *temp = (KVPair*)malloc(sizeof(KVPair));
        temp->key = parse;

        dictionary_insert(stop_dict, temp);
    }

    char throw[BUFSIZ] = "";
    fgets(throw, sizeof(throw), stdin); // reads through past the ==== delimiter between stopwords and dictionary


    // instantiating dictionaries for length and frequency, and their corresponding lists for sorting
    Dictionary* l_dict = dictionary_create(HASHTABLE_SIZE, (void*)dataPrinter, freeData);
    Dictionary* f_dict = dictionary_create(HASHTABLE_SIZE, (void*)dataPrinter, freeData);

    ListPtr freq_list = list_create(sortCompare, dataPrinter, (void*)freeData);
    ListPtr length_list = list_create(sortCompare, dataPrinter, (void*)freeData);

    // instantiating text list to retain infile read for printOutput call
    ListPtr text = list_create(sortCompare, dataPrinter, (void*)freeData);
    

    while(fgets(words, BUFSIZ, stdin)) { // reads from infile line by line
        KVPair* line = (KVPair*)malloc(sizeof(KVPair));
        line->key = strdup(words);
        list_append(text, line);
        for(char* parse = strtok(words, delim); parse; parse = strtok(NULL, delim)) {  // for each line read, tokenize and separate into KVPairs
            KVPair *wordie = (KVPair*)malloc(sizeof(KVPair));
            wordie->key = strdup(parse);


            if(dictionary_find(stop_dict, wordie->key) == NULL) { // if the stop_dict does not contain a matching pair, try to insert.
                if(dictionary_find(f_dict, wordie->key)) { // if the entry exists within the freq dictionary, then increment
                    KVPair* plus = (KVPair*)malloc(sizeof(KVPair));
                    plus->key = wordie->key;
                    

                    plus = dictionary_find(f_dict, wordie->key); 
                    plus->value = plus->value + 1;
                }
                else { // if the entry does not exist within the freq dictionary, insert into both freq and length dictionaries and their corresponding lists.
                    KVPair* freq_entry = (KVPair*)malloc(sizeof(KVPair));
                    freq_entry->key = wordie->key;
                    freq_entry->value = (void*)1;
                    dictionary_insert(f_dict, freq_entry);
                    list_append(freq_list, freq_entry);

                    KVPair* length_entry = (KVPair*)malloc(sizeof(KVPair));
                    length_entry->key = wordie->key;
                    length_entry->value = (void*)word_length(wordie->key, "\0");
                    dictionary_insert(l_dict, length_entry);
                    list_append(length_list, length_entry);
                }
            }
            
        }
    }

    // sort both lists by descending order
    list_sort(freq_list, false);
    list_sort(length_list, false);

    // delete every indices except for the top 25 for frequency, top 20 for length, while also deleting the corresponding entries in the respective dictionaries.
    while(list_length(freq_list) != 25) {
        KVPair* deletion = (KVPair*)malloc(sizeof(KVPair));
        deletion = (KVPair*)list_del_index(freq_list, 25);
        dictionary_delete(f_dict, deletion->key);
    }
    while(list_length(length_list) != 20) {
        KVPair* deletion = (KVPair*)malloc(sizeof(KVPair));
        deletion = (KVPair*)list_del_index(length_list, 20);
        dictionary_delete(l_dict, deletion->key);
    }
    
    // Use list holding original infile read and call printOutput to format text of most frequent and longest words.
    while(list_length(text) != 0) { // 
        KVPair* read = list_get_index(text, 0);
        printOutput(f_dict, l_dict, read->key);
        list_del_index(text, 0);
    }
    return 0;
}