#ifndef KDTREE_H
#define KDTREE_H

#include "helper.h"
#include "triangle.h"
#include "hittable_list.h"


/*
Class for KD tree (acceleration structure)
*/

class KDTree {

    public:

        //constructor
        KDTree(const hittable_list& world) : world(world) {}


        //destructor
        ~KDTree(){
            return;
        };


        //method to check intersection with the ray
        bool intersect(const Ray& r) const {
            return true;
        }


    private:

        hittable_list world;
        std::vector<int> tri_indices;

        //method to build the tree
        void buildTree(){
            return;
        }
};


/*
    Struct for tree node
*/

typedef struct {
    union {
        float Split; //split position used for interior nodes
        int LeafPrim; //used to store whether 0 or 1 children
        int LeafPrimOffset; //used to store the offset of the primities stored in std::vector<int> tri_indices

    };
    union {
        int Axis; //0 for x-axis, 1 for y-axis, 2 for z-axis, 3 to indicate leaf node
        int InteriorAboveSplitChild; //store index of node of right child of interior node since left child will always be stored after the parent
        int LeafPrimNum; //store number of primitives overlapping the leaf node

    };

    //methods
    void initLeaf(int np, std::vector<int>& tri_indices){
        Axis = 3;
        
    }
} Node;


#endif 