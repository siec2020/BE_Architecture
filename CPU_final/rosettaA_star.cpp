#include <list>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <chrono>

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
 
    bool isValid( point& p ) {
        return ( p.x >-1 && p.y > -1 && p.x < m.w && p.y < m.h );
    }
 
 //If we don't find a node with a cheaper path to the same point then we erase the old one and we return true else we return false and we forget the new path
    bool existPoint( point& p, int cost ) {
        list<node>::iterator i;
        i = find( closed.begin(), closed.end(), p );
        if( i != closed.end() ) {
            if( ( *i ).cost + ( *i ).dist < cost ) return true;
            else { closed.erase( i ); return false; }
        }
        i = find( open.begin(), open.end(), p );
        if( i != open.end() ) {
            if( ( *i ).cost + ( *i ).dist < cost ) return true;
            else { open.erase( i ); return false; }
        }
        return false;
    }
 
    bool fillOpen( node& n ) {
        int stepCost, nc, dist;
        point neighbour;

        //We investigate all neighbours
        for( int x = 0; x < 8; x++ ) {
            // one can make diagonals have different cost
            stepCost = x < 4 ? 1 : 1; //The variable neigbours has the direct neighbours from index 0 to 3 and the diagonal neighbours from index 4 to 7
            neighbour = n.pos + neighbours[x]; //The variable neighbours contains the relative moves from the current position to find the neighbours
            if( neighbour == end ) return true;
 
            if( isValid( neighbour ) && m( neighbour.x, neighbour.y ) != 1 ) { //Here we inspect the new position if the position is in the map and the position isn't a wall
                nc = stepCost + n.cost;
                dist = calcDist( neighbour );
                if( !existPoint( neighbour, nc + dist ) ) { //If we don't have any path to the same point in open or closed where the cost is cheaper, we create a new node in open
                    node m;
                    m.cost = nc; m.dist = dist;
                    m.pos = neighbour; 
                    m.parent = n.pos;
                    open.push_back( m );
                }
            }
        }
        return false;
    }
 
    /*
    You specify a beginning point, an end point, and a map where you want to find the cheapest way.
    It initializes all attributes from the object astar to keep these data in mind.
    We create the first node with parent 0 and current_pos the first position with a cost of zero.
    */
    bool search( point& s, point& e, map& mp ) {
        node n; end = e; start = s; m = mp;
        n.cost = 0; n.pos = s; n.parent = 0; n.dist = calcDist( s ); 
        open.push_back( n );
        while( !open.empty() ) { //Search stops when all nodes are closed, it means all ways have been inverstigated
            //open.sort();
            node n = open.front(); //FIFO research
            open.pop_front(); //As we investigated the node, we can consider it closed (i.e. investigated)
            closed.push_back( n ); //So we fill the node in closed to keep it in memory
            if( fillOpen( n ) ) return true; //
        }
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
    point s, e(x_end,y_end); //s is the start e is the end
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
