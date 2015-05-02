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
              const long unsigned int n = nnodes; 
              scoped_actor self;
              // create five chopsticks
              aout(self) << "chopstick ids are:";
              std::vector<chopstick> chopsticks;
              size_t i;
              for (i = 0; i < n; ++i) {
                chopsticks.push_back(spawn_typed(available_chopstick));
                aout(self) << " " << chopsticks.back()->id();
              }
              aout(self) << endl;
              // spawn n philosophers
              std::vector<std::string> names; 
              std::vector<caf::actor> workers;
              std::vector<uint64_t> results;
              for (i = 0; i < n; ++i) {
                names.push_back("Philosopher " + to_string( i ));
              }
              for (i = 0; i < n; ++i) {
                workers.push_back(self -> spawn<philosopher, monitored>(names[i], i*12312, chopsticks[i], chopsticks[(i + 1) % n], self));
              }
              
              Rcpp::Rcout << "Philosophers created.\n";

              i=0; 
              self -> receive_loop(
                        on<uint64_t>() >> [&](uint64_t value1){
                            results.push_back(value1);
                            aout(self) << "The host has received result: " << value1 << "\n";                       
                            if (results.size() >= n){
                                // break here?
                            }
                        },
                        others >> [&](){
                            aout(self) << "Other message received\n";                    
                        }
                      );
            
              Rcpp::Rcout << "All of the philosophers have left the building. Here are their answers: \n";
              for (i = 0; i < results.size(); i++)
              {
                  Rcpp::Rcout << "Philospher " << i << " obtained: " << results[i] << "\n";
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

