/******************************************************************************\
 * This example is an implementation of the classical Dining Philosophers     *
 * exercise using only libcaf's event-based actor implementation.             *
\ ******************************************************************************/
#include <map>
#include <vector>
#include <chrono>
#include <sstream>
#include <Rcpp.h>
#include <iostream>
#include "caf/all.hpp"
#include "philosopher.hpp"
#include <random>


using namespace Rcpp;
using namespace std;
using namespace caf;
namespace AFTestNamespace {

// Chopstick Behavior
chopstick::behavior_type taken_chopstick(chopstick::pointer self, actor_addr);
// either taken by a philosopher or available
chopstick::behavior_type available_chopstick(chopstick::pointer self) {
  return {
    [=](take_atom) {
      self->become(taken_chopstick(self, self->current_sender()));
      return taken_atom::value;
    },
    [](put_atom) {
      cerr << "chopstick received unexpected 'put'" << endl;
    }
  };
}

chopstick::behavior_type taken_chopstick(chopstick::pointer self,
                                         actor_addr user) {
  return {
    [](take_atom) {
      return busy_atom::value;
    },
    [=](put_atom) {
      if (self->current_sender() == user) {
        self->become(available_chopstick(self));
      }
    }
  };
}

philosopher::philosopher(const std::string& n, int randomSeed, const chopstick& l, const chopstick& r)
      : name(n),
        seed(randomSeed),
        left(l),
        right(r) {
    // Wake up, philosopher
    rd = new std::random_device(); // Doesn't use seed, but could instead.    
    generator = new std::mt19937((*rd)());
    mealsEaten = 0;
    thoughtsHad = 0;
    ultimateAnswer = 0;
    
    // a philosopher that receives {eat} stops thinking and becomes hungry
    thinking.assign(
      [=](eat_atom) {
        become(hungry);
        send(left, take_atom::value);
        send(right, take_atom::value);
      }
    );
    // wait for the first answer of a chopstick
    hungry.assign(
      [=](taken_atom) {
        become(granted);
      },
      [=](busy_atom) {
        become(denied);
      }
    );
    // philosopher was able to obtain the first chopstick
    granted.assign(
      [=](taken_atom) {
        aout(this) << name
                   << " has picked up chopsticks with IDs "
                   << left->id() << " and " << right->id()
                   << " and starts to eat meal " << mealsEaten << "\n";
        // eat some time
        mealsEaten += 1;
        if (mealsEaten > 10)
        {
            delayed_send(this, seconds(1), leave_atom::value);
        }
        else
        {
            delayed_send(this, seconds(1), think_atom::value);
        }
        become(eating);
      },
      [=](busy_atom) {
        send(current_sender() == left ? right : left, put_atom::value);
        send(this, eat_atom::value);
        become(thinking);
      }
    );
    // philosopher was *not* able to obtain the first chopstick
    denied.assign(
      [=](taken_atom) {
        send(current_sender() == left ? left : right, put_atom::value);
        send(this, eat_atom::value);
        become(thinking);
      },
      [=](busy_atom) {
        send(this, eat_atom::value);
        become(thinking);
      }
    );
    // philosopher obtained both chopstick and eats (for five seconds)
    eating.assign(
      [=](think_atom) {
        send(left, put_atom::value);
        send(right, put_atom::value);
        delayed_send(this, seconds(1), eat_atom::value);
        aout(this) << name << " puts down his chopsticks and starts to think\n";
        int i;
        std::poisson_distribution<> d(10);
        for (i = 0; i < 100; i++)
        {
            ultimateAnswer += d(*generator); 
        }
        become(thinking);
      },
     [=](leave_atom) {
        aout(this) << "I, " << name << ", have eaten my fill, and discovered" 
        << " that the ultimate answer is " << ultimateAnswer << "\n";
        send(left, put_atom::value);
        send(right, put_atom::value);
        quit();
      } 
    );
  }

philosopher::~philosopher()
{
    delete generator;
    delete rd;
}

behavior philosopher::make_behavior(){
    // start thinking
    send(this, think_atom::value);
    // philosophers start to think after receiving {think}
    return (
      [=](think_atom) {
        aout(this) << name << " starts to think\n";
        delayed_send(this, seconds(1), eat_atom::value);
        become(thinking);
      }
    );
  }
}

