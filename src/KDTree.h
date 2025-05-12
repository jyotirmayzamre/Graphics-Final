#ifndef KDTREE_H
#define KDTREE_H

#include "helper.h"
#include "triangle.h"
#include "hittable_list.h"
#include <variant>

/*
Reference used for KD Tree Algorithm: https://pbr-book.org/3ed-2018/Primitives_and_Intersection_Acceleration/Kd-Tree_Accelerator#
*/

/*
    Struct for bounding edge
*/

enum class EdgeType { Start, End };
struct BoundEdge {
    float t;
    int num_prims;
    EdgeType type;

    BoundEdge() {}

    BoundEdge(float t, int primNum, bool starting) : t(t), num_prims(num_prims){
        type = starting ? EdgeType::Start : EdgeType::End;
    }
};


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

/* To-Do struct*/

struct ToDo {
    const KD_Node* node;
    double tMin, tMax;
};

/*
Class for KD tree (acceleration structure)
*/

class KDTree {

    public:

        //constructor
        KDTree(const hittable_list& world, int isectCost = 80, int traversalCost = 1, int maxPrims = 1, int maxDepth = -1) : isectCost(isectCost), traversalCost(traversalCost), maxPrims(maxPrims), world(world){
            allocated_nodes = 0;
            next_free = 0;
            size_t primSize = world.size();
            if (maxDepth <= 0){
                size_t count = primSize;
                maxDepth = std::round(8 + 1.3 * std::log2(static_cast<double>(count)));
            }

            //store bounding boxes for each primitive
            std::vector<Bounds> primBounds;
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

            
            buildTree(0, bounds, primBounds, primNums, primSize, maxDepth, edges, prims1, prims2, 0);
            std::cerr << "Tree built";
        }


        //destructor
        ~KDTree(){
            return;
        };


        //method to check intersection with the ray
        bool intersect(const Ray& r, hit_record& rec) const {
            double tMin, tMax;
            if(!bounds.intersect(r, tMin, tMax)){
                return false;
            }


            vec3 invDir = 1.0 / r.direction();
            ToDo arr[64];
            int curr = 0;

            bool hit = false;
            const KD_Node* node = &nodes[0];

            while (node != nullptr){
                if (!node->isLeaf()){

                    int axis = node->splitAxis();

                    double orig = getCoord(r.origin(), axis);
                    double r_dir = getCoord(point3(r.direction()), axis);
                    double inv_dir = getCoord(invDir, axis);
                    float tPlane = (node->splitPos() - orig) * inv_dir;

                    //get children pointers
                    const KD_Node* firstChild, *secondChild;
                    int belowFirst = (orig < node->splitPos()) || (orig == node->splitPos() && r_dir <= 0);
                    if (belowFirst){
                        firstChild = node + 1;
                        secondChild = &nodes[node->aboveChild()];
                    } else {
                        firstChild = &nodes[node->aboveChild()];
                        secondChild = node + 1;
                    }

                    if (tPlane > tMax || tPlane <= 0){
                        node = firstChild;
                    } else if (tPlane < tMin){
                        node = secondChild;
                    } else {
                        arr[curr].node = secondChild;
                        arr[curr].tMin = tPlane;
                        arr[curr].tMax = tMax;
                        curr++;
                        node = firstChild;
                        tMax = tPlane;

                    }
                } else {
                    int nPrimitives = node->numPrimitives();
                    if (nPrimitives == 1){
                        const std::shared_ptr<hittable> &p = world.objects[node->one_prim];
                        if(p->hit(r, interval(tMin, tMax), rec)){
                            hit = true;
                        } else {
                            for (int i = 0; i < nPrimitives; i++){
                                hit_record temp_rec;
                                auto closest = tMax;
                                int index = tri_indices[node->index_offset + i];
                                const std::shared_ptr<hittable> &p = world.objects[index];
                                if(p->hit(r, interval(tMin, closest), temp_rec)){
                                    hit = true;
                                    closest = temp_rec.t;
                                    rec = temp_rec; 
                                }
                            }
                        }
                    }
                }

                if (curr > 0){
                    curr--;
                    node = arr[curr].node;
                    tMin = arr[curr].tMin;
                    tMax = arr[curr].tMax;
                } else{
                    break;
                }
            }
            return hit;


        }


    private:
        const int isectCost, traversalCost, maxPrims;
        hittable_list world;
        std::vector<int> tri_indices;
        std::vector<KD_Node> nodes;
        int allocated_nodes;
        int next_free;
        Bounds bounds = world.BoundingBox();
        

        //method to build the tree
        void buildTree(int node_offset, const Bounds& node_bounds, const std::vector<Bounds>& allBounds, std::vector<int>& primNums, int num_prims, int depth, std::unique_ptr<BoundEdge[]> edges[3], std::vector<int>& prims0, std::vector<int>& prims1, int badRefines){
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
            int axis = node_bounds.largest();
            int retries = 0;
            retry:

                //initialize edges
                for (int i = 0; i < num_prims; i++){
                    int prim = primNums[i];
                    const Bounds& bound = allBounds[prim];
                    float minVal = getCoord(bound.min, axis);
                    float maxVal = getCoord(bound.max, axis);
                    edges[axis][2*i] = BoundEdge(minVal, prim, true);
                    edges[axis][2*i + 1] = BoundEdge(maxVal, prim, false);
                }

                //sort edges for the given axis
                std::sort(&edges[axis][0], &edges[axis][2*num_prims],
                    [](const BoundEdge & a, const BoundEdge &b) {
                        return (a.t == b.t) ? ((int)a.type < (int)b.type) : (a.t < b.t);
                    });

                //computing costs of splits along a particular axis
                int below = 0, above = num_prims;
                for (int i = 0; i < 2 * num_prims; i++){
                    if (edges[axis][i].type == EdgeType::End){
                        above--;
                    }
                    float point = edges[axis][i].t;
                    double minVal = getCoord(node_bounds.min, axis);
                    double maxVal = getCoord(node_bounds.max, axis);
                    if (point > minVal && point < maxVal){
                        int axis1 = (axis + 1) % 3, axis2 = (axis+2)%3;
                        double d1 = getCoord(diff, axis1);
                        double d2 = getCoord(diff, axis2);
                        
                        //get surface areas of possible children
                        float bSA = 2*(d1*d2 + (point - minVal) * (d1 + d2));
                        float aSA = 2*(d1*d2 + (maxVal - point) * (d1 + d2));

                        //compute costs
                        float pBelow = bSA * invNodeSA;
                        float pAbove = aSA * invNodeSA;

                        //heuristic for cost 
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
            if (bestCost > oldCost) badRefines++;
            if (bestCost > 4*oldCost && num_prims < 16 || bestAxis == -1 || badRefines == 3){
                nodes[node_offset].initLeaf(primNums, num_prims, tri_indices);
                return;
            }

            //classify primitives according to split decided by heuristic
            int n0 = 0, n1 = 0;
            for (int i = 0; i < bestOffset; i++){
                if(edges[bestAxis][i].type == EdgeType::Start){
                    prims0[n0++] = edges[bestAxis][i].num_prims;
                }
            }
            for (int i = bestOffset + 1; i < 2 * num_prims; i++){
                if (edges[bestAxis][i].type == EdgeType::End){
                    prims1[n1++] = edges[bestAxis][i].num_prims;
                }
            }
            // std::cerr << n0 << ' ' << n1 << '\n';
            float split = edges[bestAxis][bestOffset].t;
            Bounds bounds0 = node_bounds, bounds1 = node_bounds;
            switch(bestAxis){
                case(0):
                    bounds0.max.x = split;
                    bounds1.min.x = split;
                    break;
                case(1):
                    bounds0.max.y = split;
                    bounds1.min.y = split;
                    break;
                case(2):
                    bounds0.max.z = split;
                    bounds1.min.z = split;
                    break;
                default: throw std::runtime_error("Invalid axis");
            }

            std::vector<int> tail(prims1.begin() + num_prims, prims1.end());
            //recursively build children node
            buildTree(node_offset + 1, bounds0, allBounds, prims0, n0, depth-1, edges, prims0, tail, badRefines);

            int aboveChild = next_free;
            nodes[node_offset].initInterior(bestAxis, aboveChild, split);
            buildTree(aboveChild, bounds1, allBounds, prims1, n1, depth-1, edges, prims0, tail, badRefines);
        }
};


#endif 