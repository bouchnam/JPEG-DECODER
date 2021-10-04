#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "bitstream.h"


// structure de la table de huffman
struct huff_table {
    uint32_t *lengths;
    int8_t *symbols;
    struct tree *tree;
};


// structure de l'arbre
struct tree {
    int8_t symbol;
    bool is_leaf;
    struct tree *left_children;
    struct tree *right_children;
};


// Ajoute le symbole à la profondeur correspondante
bool huffman_insert_tree(struct tree *tree, uint32_t height, uint32_t symbol)
{
    if (tree->is_leaf) {
        return false;
    }

    if (height == 1) {
        if (tree->left_children == NULL) {
            //donc on l'ajoute à au fils droite
            tree->left_children = malloc(sizeof(struct tree ));
            tree ->left_children->symbol = symbol;
            tree ->left_children->left_children = NULL;
            tree ->left_children->right_children = NULL;
            tree ->left_children->is_leaf = true;
            return true;
        } else if(tree->right_children == NULL){
            //donc on l'ajoute à au fils gauche
            tree->right_children = malloc(sizeof(struct tree ));
            tree ->right_children->symbol = symbol;
            tree ->right_children->left_children = NULL;
            tree ->right_children->right_children = NULL;
            tree ->right_children->is_leaf = true;
            return true;
        } else {
            return false;
        }

    } else if (height > 1) {

        if (tree ->left_children == NULL) {
            tree ->left_children = malloc(sizeof(struct tree ));
            tree ->left_children->symbol = 0 ;//un symbol qlq
            tree ->left_children->left_children = NULL;
            tree ->left_children->right_children = NULL;
            tree ->left_children->is_leaf = false;
            return huffman_insert_tree(tree->left_children,height-1,symbol);

        } else if (!huffman_insert_tree(tree->left_children,height-1,symbol)) {
            // on a réussit a l'insérer
            if (tree->right_children == NULL){
                tree ->right_children = malloc(sizeof(struct tree ));
                tree ->right_children->symbol = 0 ;//un symbol qlq
                tree ->right_children->left_children = NULL;
                tree ->right_children->right_children = NULL;
                tree ->right_children->is_leaf = false;
                return huffman_insert_tree(tree->right_children,height-1,symbol);
            } else {
                return huffman_insert_tree(tree->right_children,height-1,symbol);
            }

        } else {
            return true;
        }
    }

    tree -> symbol = symbol;
    tree->is_leaf = true ;
    return true;
}


// Transforme la liste des symboles en un arbre de huffman
struct tree *table_to_tree(uint32_t *table_lenghts, int8_t *symbols)
{
    struct tree *huff_tree = malloc(sizeof(struct tree));
    huff_tree ->symbol = 0 ;//un symbol qlq
    huff_tree ->left_children = NULL;
    huff_tree ->right_children = NULL;
    huff_tree ->is_leaf = false;
    uint32_t index = 0;
    for(uint32_t i = 0; i < 16; i++){
        for(uint32_t j = 0; j < table_lenghts[i]; j++){
            huffman_insert_tree(huff_tree , i+1,symbols[index + j]);
        }
        index += table_lenghts[i];
    }
    return huff_tree;
}


// parcourt le stream pour construire la table de huffman
struct huff_table *huffman_load_table(struct bitstream *stream, uint16_t *nb_byte_read)
{
    struct huff_table *huffman_table = malloc(sizeof(struct huff_table));
    huffman_table->lengths = malloc(16*sizeof(uint32_t));
    uint32_t nb_symbols = 0;
    uint32_t byte;
    //la table des longueurs
    for(uint32_t i = 0 ; i < 16; i++) {
        bitstream_read(stream, 8, &byte, false);
        (huffman_table ->lengths)[i] = byte;
        *nb_byte_read+=1;
        nb_symbols+= byte;
    }
 	// la table des symbols
	huffman_table->symbols = malloc(nb_symbols*sizeof(uint32_t));
        for(uint32_t j = 0; j < nb_symbols; j++){
            bitstream_read(stream, 8, &byte, false);
            (huffman_table ->symbols)[j] = byte;
            *nb_byte_read+=1;
        }

    // on creer l'arbre a partir des deux tables
    struct tree *tree = table_to_tree(huffman_table->lengths, huffman_table->symbols);
    huffman_table -> tree =tree ;
    return huffman_table;
}


// retourne le symbole correspondant au code lu dans le stream
int8_t huffman_next_value(struct huff_table *table, struct bitstream *stream)
{
    struct tree *tree = table->tree;
    uint32_t bit_current = 0;

    while (tree && !tree->is_leaf ) {
        bitstream_read(stream, 1, &bit_current, true);
        if (bit_current == 0) {
            if (tree->left_children == NULL) {
                fprintf(stderr, "[src/huffman.c] Invalid Huffman sequence in bitstream\n");
                exit(1);
            }
            //si le bit est 0 on decend a gauche
            tree = tree->left_children;
        } else {
            if (tree->right_children == NULL) {
                fprintf(stderr, "[src/huffman.c] Invalid Huffman sequence in bitstream\n");
                exit(1);
            }
            //si le bit est 1 on decend a droite
            tree = tree->right_children;
        }
    }
    return tree->symbol;
}


// libère la mémoire associée à l'arbre
void free_tree(struct tree *tree)
{
    if (tree != NULL) {
        free_tree(tree->left_children);
        free_tree(tree->right_children);
        free(tree);
    }
}


// libère la mémoire associée à la table de huffman
void huffman_free_table(struct huff_table *table)
{
    if (table!=NULL) {
    free_tree(table->tree);
    free(table->lengths);
    free(table->symbols);
    free(table);
    }
}


/*
void display_tree(struct tree* tree){
    if (tree->is_leaf ){
        printf("%d\n",tree->symbol);
    }
    else{
        parcours(tree->left_children);
        parcours(tree->right_children);
    }
}

int main(){
    uint32_t tab[16]= {0, 3,2,0,0};
    uint16_t tab_sym[10] = {1,2,3,4,5};
    struct huff_table *huffman_table = malloc(sizeof(struct huff_table));
    huffman_table -> lengths =malloc(16*sizeof(uint32_t));
    for(uint16_t i = 0;i<16;i++){
        huffman_table->lengths[i] = tab[i];
    //    printf("%d\n",huffman_table->lengths[i]);
    }
    huffman_table ->symbols = malloc(10*sizeof(uint32_t));
    for(uint16_t i = 0;i<10;i++){
        huffman_table->symbols[i] = tab_sym[i];
        //printf("%d\n",huffman_table->symbols[i]);
    }
    struct tree *huff_tree = table_to_tree(huffman_table -> lengths, huffman_table ->symbols);
    display_tree(huff_tree);
}
*/
