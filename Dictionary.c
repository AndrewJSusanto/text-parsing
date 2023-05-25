#include "List.h"
#include "HashTable.h"
#include "Dictionary.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Dictionary {
int		slots;				// total number of slots in hash table
int		size;				// number of elements currently in dictionary
ListPtr  *hash_table;	    // hash_table is array of ListPtrs
} Dictionary;			// dictionary is maintained as a hash table.

//TODO: Implement a key comparison function to pass to list. (This only needs to check for equality)
int dataCompare(void *obj1, void *obj2) { // NOT used for sorting. Used for inserting, finding, etc by matching
    KVPair *temp1 = (KVPair*)obj1;
    KVPair *temp2 = (KVPair*)obj2;
    int comparison = strcmp(temp1->key, temp2->key);

    return comparison;
    // strcmp returns 0 if equal, 1 if temp1 > temp2, -1 if temp2 is greater.
}


Dictionary *dictionary_create(int hash_table_size, void (*dataPrinter)(void *data), void (*freeData)(KVPair *kvPair)) { // Constructor
    Dictionary *dictionary = (Dictionary*)malloc(sizeof(Dictionary));
    dictionary->slots = hash_table_size;
    dictionary->size = 0;
    dictionary->hash_table = (ListPtr*)calloc(dictionary->slots, sizeof(ListPtr));
    unsigned int i = 0;
    for (i; i < dictionary->slots; i++) {
        dictionary->hash_table[i] = list_create(dataCompare, dataPrinter, (void*)freeData);
    }
    return dictionary;
}

void dictionary_destroy(Dictionary *d, bool freeData) { // Destructor; frees data allocated by dictionary
    for(unsigned i = 0; i < d->slots; i++) {
        list_destroy(d->hash_table[i], freeData);
    }
    free(d);
}

bool dictionary_insert(Dictionary *D, KVPair *elem) { // inserts key value pair into dictionary
    unsigned int hashkey = ht_hash(elem->key, D->slots);
    int check = list_find_element(D->hash_table[hashkey], (void*)elem); // must pass elem as void*
    
    if(check != -1) { // list_find_element returns == -1 if not found; in this case, insert
        return false; // insert could not be completed, increment completed in driver;
    }
    else { // kvpair found at index check, increment
        list_append(D->hash_table[hashkey], (void*)elem); // list_append(ListPtr, void*) // append at end of list within indices specified by hash key.
        D->size++; // increment count of # of entries in dictionary.
        return true; // insert completed.
    }
}

KVPair *dictionary_delete(Dictionary *D, char *key) { // deletes entry from dictionary and returns KVPair removed
    unsigned int hashkey = ht_hash(key, D->slots);
    KVPair* temp = (KVPair*)malloc(sizeof(KVPair));
    temp->key = key;

    int check = list_find_element(D->hash_table[hashkey], (void*)temp); // need to pass key through a KVPair object, thus must instantiate KVPair object and pass that casted as void.

    if(check != -1) { // if the list element cannot be found, return NULL.
        free(temp);
        D->size--;
        return (KVPair*)list_del_index(D->hash_table[hashkey], check); 
    }
    else if(check == -1) { // if the list element is found.
        free(temp);
        return NULL;
    }
}

// Gets the entry from the dictionary for the given key. Returns NULL if not in dictionary.
KVPair *dictionary_find(Dictionary *D, char *k) { // use char *k and hash it, go to index of hash, pull list find from that index
    unsigned int hashkey = ht_hash(k, D->slots);
    KVPair* temp = (KVPair*)malloc(sizeof(KVPair));
    temp->key = k;


    int check = list_find_element(D->hash_table[hashkey], (void*)temp);

    if(check != -1) {
        return (KVPair*)list_get_index(D->hash_table[hashkey], check);
    }
    else {
        return NULL;
    }

}

void dictionary_print(Dictionary *D) { // prints each list within nodes of dictionary
    unsigned int i = 0;
    for(i; i < D->slots; i++) {
        if(D->hash_table[i] != NULL) { 
            list_print(D->hash_table[i]); 
        }
    }
}

int dictionary_size(Dictionary* D) { // returns size of dictionary
    return D->size;
}