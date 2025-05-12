#ifndef KDTREE_H
#define KDTREE_H

#include "helper.h"
#include "triangle.h"
#include "hittable_list.h"

/*
    Struct for bounding edge
*/
enum class EdgeType { Start, End };

struct BoundEdge {
    float t;
    int primNum;
    EdgeType type;

    BoundEdge() {}
    BoundEdge(float t, int primNum, bool starting) : t(t), primNum(primNum){
        type = starting ? EdgeType::Start : EdgeType::End;
    }
};

/*
    Struct for tree node
*/

struct KD_Node {
    union {
        float split_pos; //split position used for interior nodes
        int one_prim; //used to store whether 0 or 1 children
        int index_offset; //used to store the offset of the primities stored in std::vector<int> tri_indices

    };
    union {
        int axis; //0 for x-axis, 1 for y-axis, 2 for z-axis, 3 to indicate leaf node
        int above_child; //store index of node of right child of interior node since left child will always be stored after the parent
        int num_prims; //store number of primitives overlapping the leaf node

    };

    //methods

    //method to initialize leaf node
    void initLeaf(const std::vector<int>& prim_indices, int np, std::vector<int>& tri_indices){
        axis = 3;
        num_prims |= (np << 2);
        if (np == 0){
            one_prim = 0;
        } else if (np == 1){
            one_prim = prim_indices[0];
        } else{
            index_offset = tri_indices.size();
            for(int i = 0; i < np; i++){
                tri_indices.push_back(prim_indices[i]);
            }
        }
    }

    //method to initialize interior node
    void initInterior(float split, int axis, int child_index){
        split_pos = split;
        axis = axis;
        above_child |= (child_index << 2);
    }

    //getters
    float splitPos() const {return split_pos;}
    int numPrimitives() const {return num_prims >> 2;}
    int splitAxis() const {return axis & 3;}
    bool isLeaf() const {return (axis & 3) == 3;}
    int aboveChild() const {return above_child >> 2;}
};

/*
Class for KD tree (acceleration structure)
*/

class KDTree {

    public:

        //constructor
        KDTree(const hittable_list& world, int isectCost = 80, int traversalCost = 1, float emptyBonus = 0.5, int maxPrims = 1, int maxDepth = -1) : isectCost(isectCost), traversalCost(traversalCost), maxPrims(maxPrims), emptyBonus(emptyBonus), world(world){
            allocated_nodes = 0;
            next_free = 0;
            size_t primSize = world.size();
            if (maxDepth <= 0){
                size_t count = primSize;
                maxDepth = std::round(8 + 1.3 * std::log2(static_cast<double>(count)));
            }

            //store bounding boxes for each primitive
            std::vector<Bounds3f> primBounds;
            primBounds.reserve(primSize);
            for (const auto& object : world.objects){
                primBounds.push_back(object->BoundingBox());
            }

            //create an indices vector
            std::vector<int> primNums(primSize);
            for(size_t i = 0; i < primSize; i++){
                primNums[i] = i;
            }

            //2 * primSize is needed for computing costs 
            std::unique_ptr<BoundEdge[]> edges[3];
            for (int i = 0; i < 3; i++){
                edges[i].reset(new BoundEdge[2 * primSize]);
            }

            std::vector<int> prims1(primSize);
            std::vector<int> prims2((maxDepth+1)*primSize);

            
            buildTree(0, bounds, primBounds, primNums, primSize, maxDepth, edges, prims1, prims2);
        }


        //destructor
        ~KDTree(){
            return;
        };


        //method to check intersection with the ray
        bool intersect(const Ray& r) const {
            return true;
        }


    private:
        const int isectCost, traversalCost, maxPrims;
        const float emptyBonus;
        hittable_list world;
        std::vector<int> tri_indices;
        std::vector<KD_Node> nodes;
        int allocated_nodes;
        int next_free;
        Bounds3f bounds = world.BoundingBox();
        

        //method to build the tree
        void buildTree(int node_offset, const Bounds3f& node_bounds, const std::vector<Bounds3f>& allBounds, std::vector<int>& primNums, int num_prims, int depth, const std::unique_ptr<BoundEdge[]> edges[3], std::vector<int>& prims0, std::vector<int>& prims1){
            if (next_free == allocated_nodes){
                int newNum = std::max(allocated_nodes*2, 512);
                if (nodes.size() < newNum){
                    nodes.resize(newNum);
                }
                allocated_nodes = newNum;
            }
            next_free++;

            //initialize leaf node
            if (num_prims < maxPrims || depth == 0){
                nodes[node_offset].initLeaf(primNums, num_prims, tri_indices);
            }

            //split axis
            int bestAxis = -1;
            int bestOffset = -1;
            float bestCost = infinity;
            float oldCost = isectCost * float(num_prims);
            float invNodeSA = 1/node_bounds.SurfaceArea();

            //used for calculating new surface areas
            point3 diff = node_bounds.max - node_bounds.min;

            //choose axis based on max distance
            int axis = node_bounds.maximumExtent();
            int retries = 0;
            retry:

                //initialize edges
                for (int i = 0; i < num_prims; i++){
                    int prim = primNums[i];
                    const Bounds3f& bound = allBounds[prim];
                    edges[axis][2*i] = BoundEdge(getCoord(bound.min, axis), prim, true);
                    edges[axis][2*i + 1] = BoundEdge(getCoord(bound.max, axis), prim, false);
                }

                //sort edges for the given axis
                std::sort(&edges[axis][0], &edges[axis][2*num_prims],
                    [](const BoundEdge & e0, const BoundEdge &e1) -> bool {
                        if (e0.t == e1.t){
                            return (int)e0.type < (int)e1.type;
                        } else {
                            return e0.t < e1.t;
                        }
                    });

                //computing costs of splits along a particular axis
                int below = 0, above = num_prims;
                for (int i = 0; i < 2 * num_prims; i++){
                    if (edges[axis][i].type == EdgeType::End){
                        above--;
                    }
                    float point = edges[axis][i].t;
                    if (point > getCoord(node_bounds.min, axis) && point < getCoord(node_bounds.max, axis)){
                        int axis1 = (axis + 1) % 3, axis2 = (axis+2)%3;
                        double d1 = getCoord(diff, axis1);
                        double d2 = getCoord(diff, axis2);
                        float belowSA = 2*(d1*d2 + (point - getCoord(node_bounds.min, axis)) * (d1 + d2));
                        float aboveSA = 2*(d1*d2 + (getCoord(node_bounds.max, axis) - point) * (d1 + d2));

                        //compute costs
                        float pBelow = belowSA * invNodeSA;
                        float pAbove = aboveSA * invNodeSA;
                        float cost = traversalCost + isectCost * (pBelow * below + pAbove * above);

                        //compare costs and update best variables
                        if (cost < bestCost){
                            bestCost = cost;
                            bestAxis = axis;
                            bestOffset = i;
                        }
                    }
                    if (edges[axis][i].type == EdgeType::Start){
                        below++;
                    }

                }

            //if no good splits were found, initialize leaf node
            if (bestAxis == -1 && retries < 2){
                retries++;
                //choose diff axis
                axis = (axis+1)%3;
                goto retry;
            }
            if (bestCost > 4*oldCost && num_prims < 16 || bestAxis == -1){
                nodes[node_offset].initLeaf(primNums, num_prims, tri_indices);
                return;
            }

            //classify primitives according to split decided by heuristic
            int n0 = 0, n1 = 0;
            for (int i = 0; i < bestOffset; i++){
                if(edges[bestAxis][i].type == EdgeType::Start){
                    prims0[n0++] = edges[bestAxis][i].primNum;
                }
            }
            for (int i = bestOffset + 1; i < 2 * num_prims; i++){
                if (edges[bestAxis][i].type == EdgeType::End){
                    prims1[n1++] = edges[bestAxis][i].primNum;
                }
            }

            float split = edges[bestAxis][bestOffset].t;
            Bounds3f bounds0 = node_bounds, bounds1 = node_bounds;
            switch(bestAxis){
                case(0):
                    bounds0.max.x = split;
                    bounds1.min.x = split;
                    return;
                case(1):
                    bounds0.max.y = split;
                    bounds1.min.y = split;
                    return;
                case(2):
                    bounds0.max.z = split;
                    bounds1.min.z = split;
                    return;
                default: throw std::runtime_error("Invalid axis");
            }

            std::vector<int> tail(prims1.begin() + num_prims, prims1.end());
            //recursively build children node
            buildTree(node_offset + 1, bounds0, allBounds, prims0, n0, depth-1, edges, prims0, tail);

            int aboveChild = next_free;
            buildTree(aboveChild, bounds1, allBounds, prims1, n1, depth-1, edges, prims0, tail);
        }
};





#endif 