#ifndef ACTOR_PHILOSOPHER_HEADER
#define ACTOR_PHILOSOPHER_HEADER
#include <map>
#include <vector>
#include <chrono>
#include <random>
#include <sstream>
#include <Rcpp.h>
#include <iostream>
#include "caf/all.hpp"

using std::chrono::seconds;
using namespace Rcpp;
using namespace std;
using namespace caf;

namespace AFTestNamespace {

    // atoms for chopstick interface
    using put_atom = atom_constant<atom("put")>;
    using take_atom = atom_constant<atom("take")>;
    using busy_atom = atom_constant<atom("busy")>;
    using taken_atom = atom_constant<atom("taken")>;
    using wash_atom = atom_constant<atom("wash")>;
    // atoms for philosopher interface
    using eat_atom = atom_constant<atom("eat")>;
    using think_atom = atom_constant<atom("think")>;
    using leave_atom = atom_constant<atom("leave")>;

    typedef typed_actor<replies_to<take_atom>
                ::with_either<taken_atom>
                ::or_else<busy_atom>,
                reacts_to<wash_atom>,
                reacts_to<put_atom>> chopstick;

    chopstick::behavior_type taken_chopstick(chopstick::pointer self, actor_addr);
    chopstick::behavior_type available_chopstick(chopstick::pointer self);
    chopstick::behavior_type taken_chopstick(chopstick::pointer self, actor_addr user);

    class philosopher : public event_based_actor {
        public:
            philosopher(const std::string& n, int randomSeed, const chopstick& l, const chopstick& r, actor p);  
            ~philosopher();
        protected:
            behavior make_behavior() override;
        private: 
            std::string name;       // the name of this philosopher
            std::random_device* rd;  // the philosopher's internal random device
            std::mt19937* generator; // the philosopher's internal random number generator
            chopstick   left;       // left chopstick
            chopstick   right;      // right chopstick
            behavior    thinking;   // initial behavior
            behavior    hungry;     // tries to take chopsticks
            behavior    granted;    // has one chopstick and waits for the second one
            behavior    denied;     // could not get first chopsticks
            behavior    eating;     // waits for some time, then go thinking again
            behavior    leaving;    // Ready to go, but needs some encouragement from the host.
            int seed;
            int mealsEaten;
            int thoughtsHad;
            uint64_t ultimateAnswer;
            actor parent;
    };

}


#endif
