//refer to https://software.intel.com/en-us/blogs/2007/11/08/have-a-fish-how-break-from-a-parallel-loop-in-tbb
//! Like a blocked_range, but becomes immediately empty if "stop" flag is true.
using namespace std;
template<typename Value>

class cancelable_range {

    tbb::blocked_range<Value> my_range;

    volatile bool& my_stop;

public:

    // Constructor for client code

    /** Range becomes empty if stop==true. */

    cancelable_range( int begin, int end, int grainsize, volatile bool& stop ) :

        my_range(begin,end,grainsize),

        my_stop(stop)

    {}

    //! Splitting constructor used by parallel_for

    cancelable_range( cancelable_range& r, tbb::split ) :

        my_range(r.my_range,tbb::split()),

        my_stop(r.my_stop)

    {}

    //! Cancel the range.

    void cancel() const {my_stop=true;}

    //! True if range is empty.

    /** Range is empty if there is request to cancel the range. */

    bool empty() const {return my_stop || my_range.empty();}

    //! True if range is divisible

    /** Range becomes indivisible if there is request to cancel the range. */

    bool is_divisible() const {return !my_stop && my_range.is_divisible();}

    //! Initial value in range.

    Value begin() const {return my_range.begin();}

    //! One past last value in range

    /** Note that end()==begin() if there is request to cancel the range.

        The value of end() may change asynchronously if another thread cancels the range. **/

    Value end() const {return my_stop ? my_range.begin() : my_range.end();}

};

//! Set to value that we are looking for.

tbb::atomic<int> indexSearch;

//! Some random predicate.

/** For demonstration purposes, the predicate is trivial.  In real life, this should

    be something that takes some significant time. */

/*bool ShouldBreakFromLoop( int i ) {

    return i==1234567;

}*/

//! Loop body, as a function object.
template <class T>
struct searchBody {
    const vector<T> points;
    const int id_point;
public: 
    searchBody(const vector<T> & points, const int id_point): points(points),id_point(id_point){

    }
    void operator()( const cancelable_range<int>& r ) const {
        // Iterate over subrange.  It is important that "<" be used for comparison,
        // because the value of r.end() changes to r.begin() if r is cancelled.
        for( int i=r.begin(); i<r.end(); ++i ) {
            // Do test for whether we want to break from loop early.
            if( points[i].getID() ==  id_point) {
                // Cancel the range
                
                r.cancel();
                // Record the value found.
                // If two values are found by different threads, no harm is done,
                // because we are storing into an atomic<int>.
                indexSearch = i;
            }
        }
    }
};

/*int main() {

    tbb::task_scheduler_init init;

    ValueThatWeFound = -1;

    bool stop = false;

    tbb::parallel_for( cancelable_range<int>(0,10000000,1,stop), Body() );

    std::printf("ValueThatWeFound=%d\n",int(ValueThatWeFound));

    return 0;

}*/
