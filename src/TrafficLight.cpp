#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive(){    
    // FP.5a : The method receive should use std::unique_lock<std::mutex>  
    std::unique_lock<std::mutex> uLock(_mutex);

    //_condition.wait() to wait for and receive new messages
    _condition.wait(uLock, [this] { return !_queue.empty(); });

    // pull them from the queue using move semantics.
    T msg = std::move(_queue.back());
    _queue.pop_back();

    // The received object should then be returned by the receive function.
    return msg;

    // ref from example_1.cpp in lesson "Building a Concurrent Message Queue"
}

template <typename T>
void MessageQueue<T>::send(T &&msg){
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::unique_lock<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight(){
    _currentPhase = TrafficLightPhase::red;
    _msgs = std::make_shared<MessageQueue<TrafficLightPhase>>();
}

void TrafficLight::waitForGreen(){
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while(true){
        TrafficLightPhase lastPhase = _msgs->receive();
        if (lastPhase == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase(){
    return _currentPhase;
}

void TrafficLight::simulate(){
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when 
    // the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

void TrafficLight::toggleLight(){
    _currentPhase = TrafficLightPhase((_currentPhase+1)%2);
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases(){
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    std::random_device ndrng;
    std::default_random_engine generator(ndrng());
    std::uniform_int_distribution<int> distribution(4,6);
    int randt = distribution(generator); 

    auto initt = std::chrono::system_clock::now();
    while(true){
        // measures the time between two loop cycles
        long sinceInit = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - initt).count();
        // the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // The cycle duration should be a random value between 4 and 6 seconds.
        
        if(randt <= sinceInit){
            this->toggleLight();
            // sends an update method to the message queue using move semantics
            // fp.4 use _msgs within the infinite loop to push each new TrafficLightPhase 
            // into it by calling send in conjunction with move semantics.
            TrafficLightPhase msg = _currentPhase;
            _msgs->send(std::move(_currentPhase));

            initt = std::chrono::system_clock::now();
            randt = distribution(generator); 
        }
    }
    //ref https://stackoverflow.com/questions/14391327/how-to-get-duration-as-int-millis-and-float-seconds-from-chrono
    //ref http://www.cplusplus.com/reference/chrono/duration_cast/
}

