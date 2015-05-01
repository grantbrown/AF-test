#include <Rcpp.h>
#include <philosopher.hpp>

using namespace Rcpp;
namespace AFTest{ 

class AFTest { 
    public: 
        AFTest(SEXP nodes)
        {
            Rcpp::IntegerVector inNodes(nodes);
            nnodes = inNodes[0];
        }

        int getNodes()
        {
            return nnodes;
        }

        void setNodes(int inNodes)
        {
            nnodes = inNodes;
        }

        void runPhilosophers()
        {
              dining_philosophers(nnodes);
              // real philosophers are never done
              // await_all_actors_done();
              // shutdown();
        }


    private:
        int nnodes;
};

RCPP_MODULE(AFTest){
    using namespace Rcpp ;
    class_<AFTest>( "AFTest" )
    .constructor<SEXP>()
    .property( "nodes", &AFTest::getNodes, &AFTest::setNodes, "Node count")
    .method("runPhilosophers", &AFTest::runPhilosophers);
}
}

