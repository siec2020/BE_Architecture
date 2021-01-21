#include <list>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <chrono>

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>

#include <thrust/copy.h>
#include <thrust/fill.h>
#include <thrust/sequence.h>

using namespace std; 
using namespace std::chrono;

#define SQUARE_SIDE_SIZE 8
#define WALL_PERCENTAGE 0.2         //To avoid no solution, max = 0.4
#define x_start 0                   //min= 0, max = SQUARE_SIDE_SIZE-1
#define y_start 0                   //min= 0, max = SQUARE_SIDE_SIZE-1
#define x_end 3                     //min= 0, max = SQUARE_SIDE_SIZE-1
#define y_end 8                     //min= 0, max = SQUARE_SIDE_SIZE-1

class point {
public:
    point( int a = 0, int b = 0 ) { x = a; y = b; }
    bool operator ==( const point& o ) { return o.x == x && o.y == y; }
    point operator +( const point& o ) { return point( o.x + x, o.y + y ); }
    int x, y;
};
 
class map {
public:
    map() {
        float current_random_value;

        w = h = SQUARE_SIDE_SIZE;
        for( int r = 0; r < h; r++ )
            for( int s = 0; s < w; s++ ){
                if( !( (s ==x_start  && r == y_start) || (s == x_end && r == y_end) )){
                    current_random_value = rand()/(float)RAND_MAX;
                    m[s][r] = current_random_value < WALL_PERCENTAGE ? 1 : 0;
                }
                else m[s][r] = 0;
                
                // cout << "m[" << s << "][" << r <<"] = " << m[s][r] << endl;
            }
            // cout << endl;
    }

    int operator() ( int x, int y ) { return m[x][y]; }
    int m[SQUARE_SIDE_SIZE][SQUARE_SIDE_SIZE];
    int w, h;
};
 
class node {
public:
    bool operator == (const node& o ) { return pos == o.pos; }
    bool operator == (const point& o ) { return pos == o; }
    bool operator < (const node& o ) { return dist + cost < o.dist + o.cost; }
    point pos, parent;
    int dist, cost;
};

//Fonction called from the GPU and executed by the GPU
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
__device__
bool isValid( point& p ) {
    return ( p.x >-1 && p.y > -1 && p.x < SQUARE_SIDE_SIZE && p.y < SQUARE_SIDE_SIZE );
}

__device__
int dev_calcDist( point& p, point& dev_end){
    // need a better heuristic
    int x = dev_end.x - p.x, y = dev_end.y - p.y;
    return( x * x + y * y );
}

 //If we don't find a node with a cheaper path to the same point then we erase the old one and we return true else we return false and we forget the new path
 __device__
 bool existPoint( point& p, int cost, list<node> dev_closed, list<node> dev_open) {
    list<node>::iterator i;
    i = thrust::find( dev_closed.begin(), dev_closed.end(), p );
    if( i != dev_closed.end() ) {
        if( ( *i ).cost + ( *i ).dist < cost ) return true;
        else { dev_closed.erase( i ); return false; }
    }
    i = thrust::find( dev_open.begin(), dev_open.end(), p );
    if( i != dev_open.end() ) {
        if( ( *i ).cost + ( *i ).dist < cost ) return true;
        else { dev_open.erase( i ); return false; }
    }
    return false;
}
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

class aStar {
public:
    aStar() {
        neighbours[0] = point( -1, -1 ); neighbours[1] = point(  1, -1 );
        neighbours[2] = point( -1,  1 ); neighbours[3] = point(  1,  1 );
        neighbours[4] = point(  0, -1 ); neighbours[5] = point( -1,  0 );
        neighbours[6] = point(  0,  1 ); neighbours[7] = point(  1,  0 );
    }
 
    int calcDist( point& p ){
        // need a better heuristic
        int x = end.x - p.x, y = end.y - p.y;
        return( x * x + y * y );
    }

    __global__
    void fillOpen(node* dev_n, point* dev_neighbours, int* dev_map, bool* dev_found, list<node> dev_open, list<node> dev_closed) {
        int stepCost, nc, dist;
        point neighbour;

        int i = threadIdx.x + blockIdx.x * blockDim.x;

        //We investigate all neighbours
        // one can make diagonals have different cost
        stepCost = i < 4 ? 1 : 1; //The variable neigbours has the direct neighbours from index 0 to 3 and the diagonal neighbours from index 4 to 7
        neighbour = dev_n->pos + dev_neighbours[i]; //The variable neighbours contains the relative moves from the current position to find the neighbours
        if( neighbour == end ) *dev_found = true;

        if( isValid( neighbour ) && dev_map[neighbour.x, neighbour.y] != 1 ) { //Here we inspect the new position if the position is in the map and the position isn't a wall
            nc = stepCost + dev_n->cost;
            dist = calcDist( neighbour );
            if( !existPoint( neighbour, nc + dist , dev_open, dev_closed) ) { //If we don't have any path to the same point in open or closed where the cost is cheaper, we create a new node in open
                node m;
                m.cost = nc; m.dist = dist;
                m.pos = neighbour; 
                m.parent = dev_n->pos;
                dev_open.push_back( m );
                }
        }
        *dev_found = false;
    }
 
    /*
    You specify a beginning point, an end point, and a map where you want to find the cheapest way.
    It initializes all attributes from the object astar to keep these data in mind.
    We create the first node with parent 0 and current_pos the first position with a cost of zero.
    */
    bool search( point& s, point& e, map& mp ) {

        //Allocate memory in the GPU
        point* dev_neighbours;
        point* dev_end;
        node* dev_n;
        int* dev_map;

        bool* host_found;
        bool* dev_found;

        list<node>* dev_open;
        list<node>* dev_closed;

        cudaMalloc( (void**)&dev_neighbours, 8*sizeof(point) ); //Declare the neighbours variable for the GPU
        cudaMalloc( (void**)&dev_end, sizeof(point) ); //Declare the end point for the GPU
        cudaMalloc( (void**)&dev_map, SQUARE_SIDE_SIZE*SQUARE_SIDE_SIZE*sizeof(point) ); //Declare the end point for the GPU

        //Copy values in the GPU's memory
        cudaMemcpy( dev_end, &e, sizeof(point), cudaMemcpyHostToDevice );
        cudaMemcpy( dev_neighbours, neighbours, 8*sizeof(point), cudaMemcpyHostToDevice );
        cudaMemcpy( dev_map, mp.m, SQUARE_SIDE_SIZE*SQUARE_SIDE_SIZE*sizeof(int), cudaMemcpyHostToDevice );

        node n; end = e; start = s; m = mp;
        n.cost = 0; n.pos = s; n.parent = 0; n.dist = calcDist( s ); 
        open.push_back( n );
        while( !open.empty() ) { //Search stops when all nodes are closed, it means all ways have been inverstigated
            //open.sort();
            node n = open.front(); //FIFO research
            open.pop_front(); //As we investigated the node, we can consider it closed (i.e. investigated)
            closed.push_back( n ); //So we fill the node in closed to keep it in memory
            
            //Declare the current node that will be processed by the GPU
            cudaMalloc( (void**)&dev_n, sizeof(node) );
            cudaMemcpy( dev_n, &n, sizeof(node), cudaMemcpyHostToDevice );

            //Declare the bool result in the GPU that is needed for our stop condition
            cudaMalloc( (void**)&dev_found, sizeof(bool) );

            //Create device open and close list
            cudaMalloc( (void**)&dev_open, open.size()*sizeof(node) );
            cudaMalloc( (void**)&dev_closed, closed.size()*sizeof(node) );
            cudaMemcpy( dev_open, open, open.size()*sizeof(node), cudaMemcpyHostToDevice );
            cudaMemcpy( dev_closed, closed, closed.size()*sizeof(node), cudaMemcpyHostToDevice );

            fillOpen<<<1,8>>>( dev_n, dev_neighbours, dev_map, dev_found, dev_open, dev_closed);
            
            //We update CPU's open and closed lists using the one that were modified by the kernel
            cudaMemcpy( open, dev_open, dev_open.size()*sizeof(node), cudaMemcpyDeviceToHost );
            cudaMemcpy( closed, dev_closed, dev_closed.size()*sizeof(node), cudaMemcpyDeviceToHost );
            
            //We free GPU's open and closed lists
            cudaFree(dev_open);
            cudaFree(dev_closed);

            cudaMemcpy( host_found, dev_found, sizeof(bool), cudaMemcpyDeviceToHost );
            if( *host_found ){
                //Free GPU's memory
                cudaFree(dev_found);
                cudaFree(dev_n);
                cudaFree(dev_end);
                cudaFree(dev_neighbours);
                cudaFree(dev_map);
                return true;
             }
             cudaFree(dev_n);
             cudaFree(dev_found);
        }
        //Free GPU's memory
        cudaFree(dev_end);
        cudaFree(dev_neighbours);
        cudaFree(dev_map);
        return false;
    }
 
    /*
    Recreate the path from the closed list containing all the nodes that leads to the solution
    */
    int path( list<point>& path ) {
        path.push_front( end ); //We last nodes first so at the end, the path list will be in the right order
        int cost = 1 + closed.back().cost; //We consider the last move to the end to cost 1 ????
        path.push_front( closed.back().pos );
        point parent = closed.back().parent;
 
        for( list<node>::reverse_iterator i = closed.rbegin(); i != closed.rend(); i++ ) { //We go through the entire close node list till we reach the start point
            if( ( *i ).pos == parent && !( ( *i ).pos == start ) ) {
                path.push_front( ( *i ).pos );
                parent = ( *i ).parent;
            }
        }
        path.push_front( start );
        return cost;
    }
 
    map m; point end, start;
    point neighbours[8];
    list<node> open;
    list<node> closed;

};
 

int main( int argc, char* argv[] ) {
    map m;
    point s(x_start,y_start), e(x_end,y_end); //s is the start e is the end
    aStar as;

    //Start point to measure executions time
    auto start = high_resolution_clock::now();

    if( as.search( s, e, m ) ) {
        list<point> path;
        int c = as.path( path );
        for( int y = -1; y < SQUARE_SIDE_SIZE+1; y++ ) {
            for( int x = -1; x < SQUARE_SIDE_SIZE+1; x++ ) {
                if( x < 0 || y < 0 || x > SQUARE_SIDE_SIZE-1 || y > SQUARE_SIDE_SIZE-1 || m( x, y ) == 1 )
                    cout << "w";
                else {
                    if( find( path.begin(), path.end(), point( x, y ) )!= path.end() )
                        cout << "x";
                    else cout << ".";
                }
            }
            cout << "\n";
        }
 
        cout << "\nPath cost " << c << ": ";
        for( list<point>::iterator i = path.begin(); i != path.end(); i++ ) {
            cout<< "(" << ( *i ).x << ", " << ( *i ).y << ") ";
        }
    }
    cout << "\n\n";

    // Stop point to measure executions time
    auto stop = high_resolution_clock::now();

    // Display execution time
    auto duration = duration_cast<microseconds>(stop - start);
    cout << "CPU execution time = " << duration.count() << " microseconds" <<endl;

    return 0;
}
