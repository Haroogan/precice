#include "precice/AAbstractImplementation.h"

precice::AAbstractImplementation::AAbstractImplementation(){
     _b = 0;

}

precice::AAbstractImplementation::~AAbstractImplementation(){

}

/**
 * @see Case class 
 */
 void precice::AAbstractImplementation::connectb(precice::InitializerNativeDispatcher* port){
   _b = port; 
 }
 
 void precice::AAbstractImplementation::disconnectb(){
    //delete _b;
    _b=NULL;
 }
 
void precice::AAbstractImplementation::initializeVertexesParallel(const int* vertexes, const int vertexes_len){
     // @todo Insert your code here
}
void precice::AAbstractImplementation::initializeAddressesParallel(const std::string* addresses, const int addresses_len){
     // @todo Insert your code here
}
void precice::AAbstractImplementation::mainParallel(){
     // @todo Insert your code here
}