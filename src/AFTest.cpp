#include <Rcpp.h>
#include <philosopher.hpp>
#include "caf/all.hpp"
#include <signal.h> 
#include <mutex>
#include <condition_variable>

using namespace Rcpp;
using namespace caf;

namespace AFTestNamespace{ 
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
            try
            {
              Rcpp::Rcout << "Dining Philosophers Start\n";

              scoped_actor self;
              // create five chopsticks
              aout(self) << "chopstick ids are:";
              std::vector<chopstick> chopsticks;
              for (size_t i = 0; i < nnodes; ++i) {
                chopsticks.push_back(spawn_typed(available_chopstick));
                aout(self) << " " << chopsticks.back()->id();
              }
              aout(self) << endl;
              // spawn n philosophers
              std::vector<std::string> names; 
              for (size_t i = 0; i < nnodes; ++i) {
                names.push_back("Philosopher " + to_string( i ));
              }
              for (size_t i = 0; i < nnodes; ++i) {
                spawn<philosopher>(names[i], chopsticks[i], chopsticks[(i + 1) % 5]);
              }


              // Block until all done, or interrupt received
              // Not currently working
              signal(SIGTERM, [](int signum) { Rcpp::Rcout << "SIGTERM\n";
                                             caf::shutdown(); 
                                             });
              signal(SIGKILL, [](int signum) {  Rcpp::Rcout << "SIGKILL\n";
                                             caf::shutdown(); 
                                             });

              await_all_actors_done();
              Rcpp::Rcout << "Dining Philosophers Return\n";
            }
            catch (std::exception &ex)
            {
                shutdown();
                forward_exception_to_r(ex);
            }
            catch (...){
                ::Rf_error("c++ exception (unknown reason)");
            }
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

