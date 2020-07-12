
#pragma once

#include "modules.hpp"
#include <mutex>

VM_BEGIN_MODULE(vm)

VM_EXPORT{
  template<typename T>
  class Singleton{
    private:
      static T & instance(){
        static T ins;
        return ins;
      }
      public:
      static T & get(){
        static std::once_flag a;
        std::call_once(a,[](){instance();});
        return instance();
      }
  };
}

VM_END_MODULE()
